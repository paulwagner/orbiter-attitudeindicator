#ifndef _ATTITUDEREFERENCEADI_H_
#define _ATTITUDEREFERENCEADI_H_

#include "attref.h"

typedef struct {
	double pitch;
	double bank;
	double yaw;
	double heading;
	double altitude;
	double pitchrate;
	double yawrate;
	double rollrate;
	VECTOR3 airspeed_vector;
	double tas;
	double os;
	double apoapsis;
	double periapsis;
	double apoT;
	double periT;
	double ecc;
	double inc;
} FLIGHTSTATUS;

class AttitudeReferenceADI : public AttitudeReference {
public:
  AttitudeReferenceADI(const VESSEL* vessel) : AttitudeReference(vessel){};
  FLIGHTSTATUS &GetFlightStatus();
  bool GetReferenceName(char *string, int n);

private:
	mutable FLIGHTSTATUS fs;

};

#endif