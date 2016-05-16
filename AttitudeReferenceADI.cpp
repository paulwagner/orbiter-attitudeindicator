#include "AttitudeReferenceADI.h"

FLIGHTSTATUS &AttitudeReferenceADI::GetFlightStatus() {
	fs.altitude = GetVessel()->GetAltitude();
	fs.pitch = GetVessel()->GetPitch()*DEG;
	fs.bank = GetVessel()->GetBank()*DEG;
	fs.yaw = GetVessel()->GetYaw()*DEG;
	oapiGetHeading(GetVessel()->GetHandle(), &fs.heading);
	fs.heading *= DEG;
	VECTOR3 v;
	GetVessel()->GetAngularVel(v);
	fs.pitchrate = v.x*DEG; fs.rollrate = v.z*DEG; fs.yawrate = -v.y*DEG;
	GetVessel()->GetShipAirspeedVector(fs.airspeed_vector);
	OBJHANDLE body = GetVessel()->GetApDist(fs.apoapsis);
	GetVessel()->GetPeDist(fs.periapsis);
	double body_rad = oapiGetSize(body);
	fs.apoapsis -= body_rad;
	fs.periapsis -= body_rad;
	ELEMENTS elem;
	ORBITPARAM orbitparam;
	double mjd;
	int frm = FRAME_EQU;
	if (GetMode() == 0) frm = FRAME_ECL;
	GetVessel()->GetElements(body, elem, &orbitparam, 0, frm);
	double m = oapiGetMass(body);
	fs.os = sqrt(GGRAV * m * (2 / body_rad - 1 / elem.a));
	fs.tas = GetVessel()->GetAirspeed();
	fs.apoT = orbitparam.ApT;
	fs.periT = orbitparam.PeT;
	fs.ecc = elem.e;
	fs.inc = elem.i;
	return fs;
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