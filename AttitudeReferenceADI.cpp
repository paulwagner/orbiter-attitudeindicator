#include "AttitudeReferenceADI.h"
#include <string>

AttitudeReferenceADI::AttitudeReferenceADI(const VESSEL* vessel) : AttitudeReference(vessel) {
	fs.navCnt = vessel->GetNavCount();
	fs.navCrs = (double*)malloc(sizeof(double) * fs.navCnt);
	for (int i = 0; i < fs.navCnt; i++)
		fs.navCrs[i] = 0;
}

AttitudeReferenceADI::~AttitudeReferenceADI() {
	if (fs.navCrs) delete fs.navCrs;
}

FLIGHTSTATUS &AttitudeReferenceADI::GetFlightStatus() {
	const VESSEL *v = GetVessel();
	VECTOR3 vec;
	ELEMENTS elem;
	ORBITPARAM orbitparam;
	OBJHANDLE body;
	// Body-independent parameters
	int frm = FRAME_EQU;
	if (GetMode() == 0) frm = FRAME_ECL;
	v->GetAngularVel(vec);
	fs.pitchrate = vec.x*DEG; fs.rollrate = vec.z*DEG; fs.yawrate = -vec.y*DEG;
	fs.docked = (v->DockingStatus(0) == 1);
	oapiGetHeading(v->GetHandle(), &fs.heading);
	oapiGetPitch(v->GetHandle(), &fs.pitch);
	oapiGetBank(v->GetHandle(), &fs.bank);
	// Target-relative Parameters
	NAVHANDLE navhandle = v->GetNavSource(GetNavid());
	NAVDATA ndata;
	fs.hasNavTarget = false;
	fs.navType = TRANSMITTER_NONE;
	if (navhandle) {
		oapiGetNavData(navhandle, &ndata);
		fs.navType = ndata.type;
		fs.hasNavTarget = true;

		OBJHANDLE tgtv = ndata.ids.hVessel;
		VECTOR3 tgtpos, vPos;
		oapiGetNavPos(navhandle, &tgtpos);
		v->GetGlobalPos(vPos);
		fs.navTargetRelPos = tgtpos - vPos;
		v->GetRelativeVel(tgtv, fs.navTargetRelVel);

		if (ndata.type == TRANSMITTER_IDS) {
			// Correct vessel docking port offset
			DOCKHANDLE vDh = v->GetDockHandle(0); // TODO: how to get correct docking handle of current vessel?
			if (vDh != 0) {
				VECTOR3 vDpos, vDdir, vDrot;
				v->GetDockParams(vDh, vDpos, vDdir, vDrot);
				v->GlobalRot(vDpos, vDpos);
				fs.navTargetRelPos -= vDpos;
			}
		}
		else if (ndata.type == TRANSMITTER_ILS || ndata.type == TRANSMITTER_VOR || ndata.type == TRANSMITTER_VTOL) {
			VECTOR3 dir = tmul(GetFrameRotMatrix(), unit(tgtpos - vPos));
			if (GetProjMode() == 0) fs.navBrg = atan2(dir.x, dir.z); else fs.navBrg = asin(dir.x);
			fs.navBrg = posangle(fs.navBrg);
			if (ndata.type == TRANSMITTER_ILS)
				fs.navCrs[GetNavid()] = ndata.ils.appdir;
		}
	}
	// Surface-relative parameters
	body = v->GetEquPos(fs.lon, fs.lat, fs.r);
	VECTOR3 plnt = _V(-sin(fs.lon), 0, cos(fs.lon));
	plnt *= (2 * PI * fs.r * cos(fs.lat) / oapiGetPlanetPeriod(body));
	MATRIX3 m;
	oapiGetRotationMatrix(body, &m);
	VECTOR3 plnt_v = mul(m, plnt);
	v->GetRelativeVel(body, vec);
	fs.gs = dist(plnt_v,  vec);

	//double mach = v->GetMachNumber();
	//ATMPARAM ap;
	//oapiGetAtm(v->GetHandle(), &ap);
	//double gamma = ac->gamma; // Ratio of specific heats
	double p1 = v->GetAtmPressure(); // Freestream pressure
	double d1 = v->GetAtmDensity(); // Freestream density
	//double ps = ac->p0; // Standard sea level pressure
	//double ds = ac->rho0; // Standard sea level density

	fs.ias = 0; fs.tas = 0;
	if (p1 > 10e-4) {
		fs.tas = v->GetAirspeed();
		const ATMCONST *ac = oapiGetPlanetAtmConstants(body);
		if (ac != 0 && ac->rho0 != 0 && d1 != 0) {
			fs.ias = fs.tas / (ac->rho0 / d1); // Approximation
		}
	}
	//if (p1 != 0 && ps != 0 && ds != 0 && gamma > 1) {
		//double as = sqrt(gamma * ps / ds); // Sea level speed of sound
		//double gamma_1 = gamma - 1;
		//double gamma_r = gamma_1 / gamma;
		//double k = 2. / gamma_1;

		// TAS
		//double p0 = p1 / (pow((mach*mach / k) + 1, 1 / gamma_r));
		//fs.tas = v->GetAirspeed();

		//if (p1 / p0 > 1)
			//fs.tas = sqrt((2*gamma*ac->R*ap.T/gamma_1) * (pow(p1/p0, gamma_r) - 1));
		//fs.tas = v->GetAirspeed();

		// IAS
		//fs.ias = as * sqrt(k * (pow(((p1 - p0) / ps) + 1, gamma_r) - 1));
		//fs.ias = fs.tas / (ds / v->GetAtmDensity());
		//fs.ias = fs.tas / (59.05 * (1 + 0.00002*3.28084*v->GetAltitude()));
	//}
	//return fs;

	// Body-relative parameters
	body = GetVessel()->GetGravityRef();
	if (GetMode() == 3 || 
		(GetMode() == 4 && navhandle != 0 && (ndata.type == TRANSMITTER_ILS || ndata.type == TRANSMITTER_VOR || ndata.type == TRANSMITTER_VTOL)))
		body = GetVessel()->GetSurfaceRef();
	double body_rad = oapiGetSize(body);
	v->GetElements(body, elem, &orbitparam, 0, frm);
	fs.apoapsis = orbitparam.ApD;
	fs.periapsis = orbitparam.PeD;
	fs.apoapsis -= body_rad;
	fs.periapsis -= body_rad;
	fs.apoT = orbitparam.ApT;
	fs.periT = orbitparam.PeT;
	fs.t = orbitparam.T;
	fs.lan = elem.theta;
	fs.ecc = elem.e;
	fs.inc = elem.i;
	v->GetRelativeVel(body, vec);
	fs.os = length(vec);
	v->GetRelativePos(body, vec);
	fs.altitude = length(vec) - body_rad;

	return fs;
}

bool AttitudeReferenceADI::GetAirspeedDirection(VECTOR3 &prograde, VECTOR3 &normal, VECTOR3 &radial, VECTOR3 &perpendicular) {
	MATRIX3 m = GetFrameRotMatrix();
	const VESSEL *v = GetVessel();
	v->GetShipAirspeedVector(prograde);
	v->GlobalRot(prograde, prograde);
	prograde = tmul(m, unit(prograde));
	v->GetRelativePos(v->GetGravityRef(), radial);
	radial = tmul(m, unit(radial));
	normal = crossp(unit(prograde), unit(radial));
	perpendicular = -crossp(unit(prograde), unit(normal));
	return true;
}

bool AttitudeReferenceADI::GetOrbitalSpeedDirection(VECTOR3 &prograde, VECTOR3 &normal, VECTOR3 &radial, VECTOR3 &perpendicular) {
	MATRIX3 m = GetFrameRotMatrix();
	const VESSEL *v = GetVessel();
	prograde = _V(0,0,1);
	perpendicular = _V(0,1,0);
	normal = _V(-1,0,0);
	v->GetRelativePos(v->GetGravityRef(), radial);
	radial = tmul(m, unit(radial));
	return true;
}

bool AttitudeReferenceADI::GetTargetDirections(VECTOR3 &tgtpos, VECTOR3 &tgtvel) {
	VECTOR3 euler;
	SetTgtmode(2);
	GetTgtEulerAngles(euler);
	CalculateDirection(euler, tgtpos);
	SetTgtmode(3);
	PostStep(0, 0, 0);
	GetTgtEulerAngles(euler);
	CalculateDirection(euler, tgtvel);
	return true;
}

bool AttitudeReferenceADI::GetDockingPortDirection(VECTOR3& dockPort) {
	DOCKHANDLE vDh = GetVessel()->GetDockHandle(0); // TODO: how to get correct docking handle of current vessel?
	if (vDh != 0) {
		MATRIX3 srot;
		GetVessel()->GetRotationMatrix(srot);
		dockPort = _V(srot.m13, srot.m23, srot.m33);

		VECTOR3 vDpos, vDdir, vDrot;
		GetVessel()->GetDockParams(vDh, vDpos, vDdir, vDrot);
		GetVessel()->GlobalRot(vDpos, vDpos);
		dockPort += vDpos;
		dockPort = tmul(GetFrameRotMatrix(), unit(dockPort));
		normalise(dockPort);
		return true;
	}
	return false;
}

void AttitudeReferenceADI::CalculateDirection(VECTOR3 euler, VECTOR3 &dir) {
	double sint = sin(euler.y), cost = cos(euler.y);
	double sinp = sin(euler.z), cosp = cos(euler.z);
	if (GetProjMode() == 0)
		dir = _V(RAD*sinp*cost, RAD*sint, RAD*cosp*cost);
	else
		dir = _V(-RAD*sint*cosp, RAD*sinp, RAD*cost*cosp);
}

bool AttitudeReferenceADI::GetReferenceName(char *string, int n) {
	OBJHANDLE handle = 0;
	switch (GetMode()) {
	case 0:
	case 1:
	case 2: {
		handle = GetVessel()->GetGravityRef();
		oapiGetObjectName(handle, string, n);
	}
		return true;
	case 3: {
		handle = GetVessel()->GetSurfaceRef();
		oapiGetObjectName(handle, string, n);
	}
		return true;
	case 4: {
		NAVDATA ndata;
		NAVHANDLE navhandle = GetVessel()->GetNavSource(GetNavid());
		if (navhandle) {
			oapiGetNavData(navhandle, &ndata);
			switch (ndata.type) {
			case TRANSMITTER_ILS: {
				std::string s = "ILS Rwy ";
				int r = (int)(ndata.ils.appdir*DEG/10);
				if (r < 10) s.append("0");
				s.append(std::to_string(r));
				strncpy(string, s.c_str(), n);
			}	return true;
			case TRANSMITTER_VTOL: {
				std::string s = "VTOL Pad-";
				int p = ndata.vtol.npad + 1;
				if (p < 10) s.append("0");
				s.append(std::to_string(p));
				strncpy(string, s.c_str(), n);
			} return true;
			default: {
				oapiGetNavDescr(navhandle, string, n);
			} return true;
			}
		}
		else {
			strncpy(string, "[no signal]", n);
			return true;
		}
	}
	}
	return false;
}