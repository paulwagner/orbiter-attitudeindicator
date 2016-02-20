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
} FLIGHTSTATUS;

class AttitudeReferenceADI : public AttitudeReference {
public:
  AttitudeReferenceADI(const VESSEL* vessel) : AttitudeReference(vessel){};
  FLIGHTSTATUS &GetFlightStatus();

private:
	mutable FLIGHTSTATUS fs;

};

#endif