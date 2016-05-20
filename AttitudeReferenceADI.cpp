#include "AttitudeReferenceADI.h"

FLIGHTSTATUS &AttitudeReferenceADI::GetFlightStatus() {
	ELEMENTS elem;
	ORBITPARAM orbitparam;
	int frm = FRAME_EQU;
	if (GetMode() == 0) frm = FRAME_ECL;
	OBJHANDLE body;
	const VESSEL *v = GetVessel();
	fs.altitude = v->GetAltitude();
	fs.pitch = v->GetPitch()*DEG;
	fs.bank = v->GetBank()*DEG;
	fs.yaw = v->GetYaw()*DEG;
	oapiGetHeading(v->GetHandle(), &fs.heading);
	fs.heading *= DEG;
	VECTOR3 vec;
	v->GetAngularVel(vec);
	fs.pitchrate = vec.x*DEG; fs.rollrate = vec.z*DEG; fs.yawrate = -vec.y*DEG;
	NAVHANDLE navhandle = v->GetNavSource(GetNavid());
	fs.navTarget = 0;
	if (navhandle) {
		NAVDATA ndata;
		oapiGetNavData(navhandle, &ndata);
		if (ndata.type == TRANSMITTER_IDS) {
			fs.navTarget = oapiGetVesselInterface(ndata.ids.hVessel);
			v->GetRelativePos(ndata.ids.hVessel, fs.navTargetRelPos);
			v->GetRelativeVel(ndata.ids.hVessel, fs.navTargetRelVel);
			body = fs.navTarget->GetApDist(fs.navTargetAp);
			fs.navTarget->GetPeDist(fs.navTargetPe);
			double body_rad = oapiGetSize(body);
			fs.navTargetAp -= body_rad;
			fs.navTargetPe -= body_rad;
			fs.navTarget->GetElements(body, elem, &orbitparam, 0, frm);
			fs.navTargetInc = elem.i;
		}
	}
	body = v->GetApDist(fs.apoapsis);
	v->GetPeDist(fs.periapsis);
	double body_rad = oapiGetSize(body);
	fs.apoapsis -= body_rad;
	fs.periapsis -= body_rad;
	v->GetElements(body, elem, &orbitparam, 0, frm);
	double m = oapiGetMass(body);
	fs.os = sqrt(GGRAV * m * (2 / body_rad - 1 / elem.a));
	fs.tas = v->GetAirspeed();
	fs.apoT = orbitparam.ApT;
	fs.periT = orbitparam.PeT;
	fs.t = orbitparam.T;
	fs.lan = elem.theta;
	fs.ecc = elem.e;
	fs.inc = elem.i;
	v->GetEquPos(fs.lon, fs.lat, fs.r);
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
			case TRANSMITTER_IDS: {
				VESSEL *vtgt = oapiGetVesselInterface(ndata.ids.hVessel);
				strncpy(string, vtgt->GetName(), n);
			}	return true;
			case TRANSMITTER_VTOL:
			case TRANSMITTER_VOR: {
				handle = GetVessel()->GetSurfaceRef();
				oapiGetObjectName(handle, string, n);
			} return true;
			}
		}
	}
	}
	return false;
}