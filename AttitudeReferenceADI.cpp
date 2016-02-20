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
	return fs;
}