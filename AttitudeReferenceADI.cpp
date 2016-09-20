#include "AttitudeReferenceADI.h"
#include <string>
#include "commons.h"
#include "AttitudeRetrieval.h"

AttitudeReferenceADI::AttitudeReferenceADI(const VESSEL* vessel, const MFDSettings* settings) {
	v = vessel;
	s = settings;
	fs.navType = TRANSMITTER_NONE;
	prevGS = 0; prevIAS = 0; prevTAS = 0; prevOS = 0; prevAlt = 0; prevt = 0;
	PostStep(0, 0, 0);

	prevNavPos = _V(0, 0, 0);
	prevVPos = _V(0, 0, 0);
	prevst = 0;
}

AttitudeReferenceADI::~AttitudeReferenceADI() {
}

void AttitudeReferenceADI::UpdateFrameRotMatrix() {
	// Calculates rotation matrix for rotation from reference frame to global frame
	NAVDATA ndata;

	VECTOR3 axis1, axis2, axis3;
	switch (s->frm) {
	case 0:    // inertial (ecliptic)
		axis3 = _V(1, 0, 0);
		axis2 = _V(0, 1, 0);
		break;
	case 1: {  // inertial (equator)
		MATRIX3 R;
		oapiGetPlanetObliquityMatrix(v->GetGravityRef(), &R);
		axis3 = _V(R.m11, R.m21, R.m31);
		axis2 = _V(R.m12, R.m22, R.m32);
	} break;
	case 2: {  // orbital velocity / orbital momentum vector
		OBJHANDLE hRef = v->GetGravityRef();
		v->GetRelativeVel(hRef, axis3);
		axis3 = unit(axis3);
		VECTOR3 vv, vm;
		v->GetRelativePos(hRef, vv);    // local vertical
		vm = crossp(axis3, vv);    // direction of orbital momentum
		axis2 = unit(crossp(vm, axis3));
	} break;
	case 3: {  // local horizon / local north (surface)
		OBJHANDLE hRef = v->GetSurfaceRef();
		v->GetRelativePos(hRef, axis2);
		axis2 = unit(axis2);
		MATRIX3 prot;
		oapiGetRotationMatrix(hRef, &prot);
		VECTOR3 paxis = { prot.m12, prot.m22, prot.m32 };  // planet rotation axis in global frame
		VECTOR3 yaxis = unit(crossp(paxis, axis2));      // direction of yaw=+90 pole in global frame
		axis3 = crossp(axis2, yaxis);
	} break;
	case 4: {  // synced to NAV source (type-specific)
		NAVHANDLE hNav = v->GetNavSource(s->navId);
		axis3 = _V(0, 0, 1);
		axis2 = _V(0, 1, 0);
		if (hNav) {
			oapiGetNavData(hNav, &ndata);
			switch (ndata.type) {
			case TRANSMITTER_IDS: {
				VECTOR3 pos, dir, rot;
				MATRIX3 R;
				VESSEL *vtgt = oapiGetVesselInterface(ndata.ids.hVessel);
				vtgt->GetRotationMatrix(R);
				vtgt->GetDockParams(ndata.ids.hDock, pos, dir, rot);
				axis3 = -mul(R, dir);
				axis2 = mul(R, rot);
			} break;
			case TRANSMITTER_XPDR: {
				MATRIX3 R;
				VESSEL *vtgt = oapiGetVesselInterface(ndata.ids.hVessel);
				vtgt->GetRotationMatrix(R);
				axis3 = _V(R.m13, R.m23, R.m33);
				axis2 = _V(R.m12, R.m22, R.m32);
			} break;
			default: {
				// Local heading in VTOL/VOR/ILS mode
				OBJHANDLE hRef = v->GetSurfaceRef();
				v->GetRelativePos(hRef, axis2);
				axis2 = unit(axis2);
				MATRIX3 prot;
				oapiGetRotationMatrix(hRef, &prot);
				VECTOR3 paxis = { prot.m12, prot.m22, prot.m32 };  // planet rotation axis in global frame
				VECTOR3 yaxis = unit(crossp(paxis, axis2));      // direction of yaw=+90 pole in global frame
				axis3 = crossp(axis2, yaxis);
			} break;
			}
		}
		else {
			// Local heading in if no station is tuned
			OBJHANDLE hRef = v->GetSurfaceRef();
			v->GetRelativePos(hRef, axis2);
			axis2 = unit(axis2);
			MATRIX3 prot;
			oapiGetRotationMatrix(hRef, &prot);
			VECTOR3 paxis = { prot.m12, prot.m22, prot.m32 };  // planet rotation axis in global frame
			VECTOR3 yaxis = unit(crossp(paxis, axis2));      // direction of yaw=+90 pole in global frame
			axis3 = crossp(axis2, yaxis);
		}

	} break;
	}

	axis1 = crossp(axis2, axis3);
	R = _M(axis1.x, axis2.x, axis3.x, axis1.y, axis2.y, axis3.y, axis1.z, axis2.z, axis3.z);
}

void AttitudeReferenceADI::UpdateEulerAngles() {
	// Rotation matrix ship->global
	MATRIX3 srot;
	v->GetRotationMatrix(srot);

	// map ship's local axes into reference frame
	VECTOR3 shipx = { srot.m11, srot.m21, srot.m31 };
	VECTOR3 shipy = { srot.m12, srot.m22, srot.m32 };
	VECTOR3 shipz = { srot.m13, srot.m23, srot.m33 };

	NAVHANDLE navhandle = v->GetNavSource(s->navId);
	NAVDATA ndata;
	if (s->frm == 4 && s->idsDockRef && navhandle) {
		oapiGetNavData(navhandle, &ndata);
		// map ship's docking port axes into reference frame
		if (ndata.type == TRANSMITTER_IDS) {
			DOCKHANDLE vDh = v->GetDockHandle(0);
			VECTOR3 vDpos, vDrot, vDdir;
			v->GetDockParams(vDh, vDpos, vDdir, vDrot);
			v->GlobalRot(vDdir, vDdir);
			v->GlobalRot(vDrot, vDrot);
			shipz = unit(vDdir);
			shipy = unit(vDrot);
			shipx = crossp(shipy, shipz);
		}
		// swap ship's y and z axis
		if (ndata.type == TRANSMITTER_VTOL) {
			VECTOR3 tmp;
			tmp = shipy;
			shipy = shipz;
			shipz = -tmp;
		}
	}
	shipx = tmul(R, shipx);
	shipy = tmul(R, shipy);
	shipz = tmul(R, shipz);

	CheckRange(shipz.y, -1., 1.); // asin takes [-1,1]
	euler.x = atan2(shipx.y, shipy.y);  // roll angle
	euler.y = asin(shipz.y);            // pitch angle
	euler.z = atan2(shipz.x, shipz.z);  // yaw angle
	for (int i = 0; i < 3; i++)
		euler.data[i] = posangle(euler.data[i]);
}

inline void offsetDir(VECTOR3 &vec) {
	const double offs = 0.1;
	for (int i = 0; i < 3; i++) {
		if (abs(vec.data[i]) <= offs)
			vec.data[i] = 1.0 / 10000000;
		else if (vec.data[i] < 0) vec.data[i] += offs;
		else vec.data[i] -= offs;
	}
}

void AttitudeReferenceADI::UpdateTargetEulerAngles() {
	NAVHANDLE hNav;
	if (s->frm < 4 || !(hNav = v->GetNavSource(s->navId))) return;

	NAVDATA ndata;
	oapiGetNavData(hNav, &ndata);

	// Calculate euler angles for direction of NAV source
	VECTOR3 navPos, vPos;
	oapiGetNavPos(hNav, &navPos);
	v->GetGlobalPos(vPos);

	VECTOR3 relDir = navPos - vPos;
	// Correct docking port poosition in IDS mode
	if (s->idsDockRef && ndata.type == TRANSMITTER_IDS) {
		DOCKHANDLE vDh = v->GetDockHandle(0);
		VECTOR3 vDpos, vDrot, vDdir;
		v->GetDockParams(vDh, vDpos, vDdir, vDrot);
		v->GlobalRot(vDpos, vDpos);
		vPos += vDpos;
		MATRIX3 vrotm;
		v->GetRotationMatrix(vrotm);
		VECTOR3 vesselDir = tmul(vrotm, navPos - vPos);
		offsetDir(vesselDir);
		relDir = mul(vrotm, vesselDir);
	}
	else if (s->idsDockRef && ndata.type == TRANSMITTER_VTOL) {
		// Correct target orientation
		relDir *= -1;
	}
	VECTOR3 dir = tmul(R, unit(relDir));
	if (abs(relDir.x) <= 1.0 / 1000000 && abs(relDir.y) <= 1.0 / 1000000 && abs(relDir.z) <= 1.0 / 1000000) {
		dir = _V(0, 0, 1); // When close, lock on target
	}

	tgteuler.y = asin(dir.y);
	tgteuler.z = atan2(dir.x, dir.z);
	tgteuler.y = posangle(tgteuler.y);
	tgteuler.z = posangle(tgteuler.z);

	// Calculate velocity of NAV source
	VECTOR3 navVel, vVel;
	v->GetGlobalVel(vVel);
	OBJHANDLE hObj = NULL;
	switch (ndata.type) {
	case TRANSMITTER_IDS:
		hObj = ndata.ids.hVessel;
		break;
	case TRANSMITTER_XPDR:
		hObj = ndata.xpdr.hVessel;
		break;
	case TRANSMITTER_VTOL:
		hObj = ndata.vtol.hBase;
		break;
	case TRANSMITTER_VOR:
		hObj = ndata.vor.hPlanet;
		break;
	}
	if (hObj) {
		if (ndata.type == TRANSMITTER_VOR) {
			MATRIX3 Rp;
			oapiGetRotationMatrix(hObj, &Rp);
			oapiGetGlobalVel(hObj, &navVel);
			navVel += mul(Rp, _V(-sin(ndata.vor.lng), 0, cos(ndata.vor.lng)) * PI2 / oapiGetPlanetPeriod(hObj)*oapiGetSize(hObj)*cos(ndata.vor.lat));
		}
		else if (s->idsDockRef && ndata.type == TRANSMITTER_IDS) {
			double st = oapiGetSimTime();
			if (st - prevst > 0.1) {
				navVel = (navPos - prevNavPos) / (st - prevst); prevNavPos = navPos;
				vVel = (vPos - prevVPos) / (st - prevst); prevVPos = vPos;
				prevst = st;
			}
		}
		else {
			oapiGetGlobalVel(hObj, &navVel);
		}
		dir = tmul(R, unit(vVel - navVel));

		tgtveleuler.y = asin(dir.y);
		tgtveleuler.z = atan2(dir.x, dir.z);
		tgtveleuler.y = posangle(tgtveleuler.y);
		tgtveleuler.z = posangle(tgtveleuler.z);
	}
}


bool AttitudeReferenceADI::PostStep(double simt, double simdt, double mjd){
	UpdateFrameRotMatrix();
	UpdateEulerAngles();
	UpdateTargetEulerAngles();

	VECTOR3 vec;
	ELEMENTS elem;
	ORBITPARAM orbitparam;
	OBJHANDLE body;
	// Body-independent parameters
	int frm = FRAME_EQU;
	if (s->frm == 0) frm = FRAME_ECL;
	v->GetAngularVel(vec);
	fs.pitchrate = vec.x*DEG; fs.rollrate = vec.z*DEG; fs.yawrate = -vec.y*DEG;
	fs.aoa = v->GetAOA();
	fs.docked = (v->DockingStatus(0) == 1);
	fs.ground = v->GroundContact();
	oapiGetHeading(v->GetHandle(), &fs.heading);
	oapiGetPitch(v->GetHandle(), &fs.pitch);
	oapiGetBank(v->GetHandle(), &fs.bank);
	// Target-relative Parameters
	NAVHANDLE navhandle = v->GetNavSource(s->navId);
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
			VECTOR3 dir = tmul(R, unit(tgtpos - vPos));
			fs.navBrg = atan2(dir.x, dir.z);
			fs.navBrg = posangle(fs.navBrg);
			if (ndata.type == TRANSMITTER_ILS)
				s->navCrs[s->navId] = ndata.ils.appdir;
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
	body = v->GetGravityRef();
	if (s->frm == 3 ||
		(s->frm == 4 && navhandle != 0 && (ndata.type == TRANSMITTER_ILS || ndata.type == TRANSMITTER_VOR || ndata.type == TRANSMITTER_VTOL)))
		body = v->GetSurfaceRef();
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
	v->GetShipAirspeedVector(prograde);
	v->GlobalRot(prograde, prograde);
	prograde = tmul(R, unit(prograde));
	v->GetRelativePos(v->GetGravityRef(), radial);
	radial = tmul(R, unit(radial));
	normal = crossp(unit(prograde), unit(radial));
	perpendicular = -crossp(unit(prograde), unit(normal));
	return true;
}

bool AttitudeReferenceADI::GetOrbitalSpeedDirection(VECTOR3 &prograde, VECTOR3 &normal, VECTOR3 &radial, VECTOR3 &perpendicular) {
	prograde = _V(0,0,1);
	perpendicular = _V(0,1,0);
	normal = _V(-1,0,0);
	v->GetRelativePos(v->GetGravityRef(), radial);
	radial = tmul(R, unit(radial));
	return true;
}

bool AttitudeReferenceADI::GetTargetDirections(VECTOR3 &tgtpos, VECTOR3 &tgtvel) {
	CalculateDirection(tgteuler, tgtpos);
	CalculateDirection(tgtveleuler, tgtvel);
	return true;
}

bool AttitudeReferenceADI::GetManeuverDirections(VECTOR3 &man) {
	if (!s->hasManRot) return false;
	man = _V(0, 0, 1);
	MATRIX3 m;
	getRotMatrix(s->manRot, &m);
	man = tmul(m, man); // Apply maneuver rotation
	man = tmul(R, man); // Apply frame rotation
	return true;
}

void AttitudeReferenceADI::CalculateDirection(VECTOR3 euler, VECTOR3 &dir) {
	double sint = sin(euler.y), cost = cos(euler.y);
	double sinp = sin(euler.z), cosp = cos(euler.z);
	dir = _V(RAD*sinp*cost, RAD*sint, RAD*cosp*cost);
}

bool AttitudeReferenceADI::GetReferenceName(char *string, int n) {
	OBJHANDLE handle = 0;
	switch (s->frm) {
	case 0:
	case 1:
	case 2: {
		handle = v->GetGravityRef();
		oapiGetObjectName(handle, string, n);
	}
		return true;
	case 3: {
		handle = v->GetSurfaceRef();
		oapiGetObjectName(handle, string, n);
	}
		return true;
	case 4: {
		NAVDATA ndata;
		NAVHANDLE navhandle = v->GetNavSource(s->navId);
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