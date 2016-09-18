#include "AttitudeReferenceADI.h"
#include <string>
#include "commons.h"
#include "AttitudeRetrieval.h"

AttitudeReferenceADI::AttitudeReferenceADI(const VESSEL* vessel) : AttitudeReference(vessel) {
	fs.navCnt = vessel->GetNavCount();
	fs.navCrs = (double*)malloc(sizeof(double) * fs.navCnt);
	for (int i = 0; i < fs.navCnt; i++)
		fs.navCrs[i] = 0;
	fs.navType = TRANSMITTER_NONE;
	prevGS = 0; prevIAS = 0; prevTAS = 0; prevOS = 0; prevAlt = 0;
	PostStep(0, 0, 0);
}

AttitudeReferenceADI::~AttitudeReferenceADI() {
	if (fs.navCrs) delete fs.navCrs;
}

void AttitudeReferenceADI::saveCurrentAttitude() {
	fs.hasManRot = true;
	GetVessel()->GetGlobalOrientation(fs.manRot);
}

bool AttitudeReferenceADI::getExternalAttitude() {
	VECTOR3 globRot;
	fs.hasManRot = false;
	if (AttitudeRetrieval::isSupported() && AttitudeRetrieval::getExternalAttitude(globRot)) {
		fs.hasManRot = true;
		fs.manRot = globRot;
	}
	return fs.hasManRot;
}

bool AttitudeReferenceADI::PostStep(double simt, double simdt, double mjd){
	AttitudeReference::PostStep(simt, simdt, mjd);

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
	fs.aoa = v->GetAOA();
	fs.docked = (v->DockingStatus(0) == 1);
	fs.ground = v->GroundContact();
	oapiGetHeading(v->GetHandle(), &fs.heading);
	oapiGetPitch(v->GetHandle(), &fs.pitch);
	oapiGetBank(v->GetHandle(), &fs.bank);
	// Target-relative Parameters
	NAVHANDLE navhandle = v->GetNavSource(GetNavid());
	NAVDATA ndata;
	fs.hasNavTarget = false;
	DWORD prevNavType = fs.navType;
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
	bool navTypeChanged = (prevNavType != fs.navType);

	// Surface-relative parameters
	body = v->GetEquPos(fs.lon, fs.lat, fs.r);
	VECTOR3 plnt = _V(-sin(fs.lon), 0, cos(fs.lon));
	plnt *= (2 * PI * fs.r * cos(fs.lat) / oapiGetPlanetPeriod(body));
	MATRIX3 m;
	oapiGetRotationMatrix(body, &m);
	VECTOR3 plnt_v = mul(m, plnt);
	v->GetRelativeVel(body, vec);
	fs.gs = dist(plnt_v,  vec);
	
	fs.dns = v->GetAtmDensity();
	fs.stp = v->GetAtmPressure();
	fs.dnp = 0.5*fs.dns*pow(v->GetAirspeed(), 2.0);

	// IAS calculation by Hielor
	fs.ias = -1; fs.tas = -1;
	// speed of sound at sea level
	double speedOfSound = 340.29;
	// use Orbiter's constant for earth sea level pressure
	double seaLevelPres = ATMP;
	double statPres, dynPres, mach;
	const ATMCONST *atmConst;
	OBJHANDLE atmRef = v->GetAtmRef();
	if (atmRef != NULL) {
		// Freestream static pressure
		statPres = v->GetAtmPressure();

		// Retrieve the ratio of specific heats
		atmConst = oapiGetPlanetAtmConstants(atmRef);
		double gamma = atmConst->gamma;

		// Mach number
		mach = v->GetMachNumber();

		// Determine the dynamic pressure using the
		// thermal definition for stagnation pressure
		dynPres = (gamma - 1) * pow(mach, 2.0) / 2 + 1;
		dynPres = pow(dynPres, gamma / (gamma - 1));
		// Convert stagnation pressure to dynamic pressure
		dynPres = dynPres * statPres - statPres;

		if (fs.dnp > 10e-4 || fs.ground) {
			fs.tas = v->GetAirspeed();

			// Following is the equation from the Orbiter manual, page 62
			fs.ias = dynPres / seaLevelPres + 1;
			fs.ias = pow(fs.ias, ((gamma - 1) / gamma)) - 1.0;
			fs.ias = fs.ias * 2 / (gamma - 1);
			fs.ias = sqrt(fs.ias) * speedOfSound;
		}

	}

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
	if (elem.a < 0) {
		fs.t = fs.apoT = fs.apoapsis = NAN;
	}
	fs.lan = elem.theta;
	fs.ecc = elem.e;
	fs.inc = elem.i;
	v->GetRelativeVel(body, vec);
	fs.os = length(vec);
	v->GetRelativePos(body, vec);
	fs.altitude = length(vec) - body_rad;
#ifndef ORBITER2016
	fs.altitudeGround = fs.altitude;
#else
	oapiGetAltitude(v->GetHandle(), ALTMODE_GROUND, &fs.altitudeGround);
#endif

	if (simt - prevt > 0.1) {
		fs.iasAcc = (fs.ias - prevIAS) / (simt - prevt); prevIAS = fs.ias;
		fs.tasAcc = (fs.tas - prevTAS) / (simt - prevt); prevTAS = fs.tas;
		fs.osAcc = (fs.os - prevOS) / (simt - prevt); prevOS = fs.os;
		fs.gsAcc = (fs.gs - prevGS) / (simt - prevt); prevGS = fs.gs;
		fs.vs = (fs.altitude - prevAlt) / (simt - prevt); prevAlt = fs.altitude;
		prevt = simt;
	}

	return navTypeChanged;
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

bool AttitudeReferenceADI::GetManeuverDirections(VECTOR3 &man) {
	if (!fs.hasManRot) return false;
	man = _V(0, 0, 1);
	MATRIX3 R;
	getRotMatrix(fs.manRot, &R);
	man = tmul(R, man); // Apply maneuver rotation
	man = tmul(GetFrameRotMatrix(), man); // Apply frame rotation
	return true;
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