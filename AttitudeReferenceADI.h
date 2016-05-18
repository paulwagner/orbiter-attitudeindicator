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
	//bool hasTarget;
	//VECTOR3 target_vector;
	VESSEL* target;
	VECTOR3 target_dockpos;
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
  bool GetOrbitalSpeedDirection(VECTOR3 &prograde, VECTOR3 &normal, VECTOR3 &radial, VECTOR3 &perpendicular);
  bool GetAirspeedDirection(VECTOR3 &prograde, VECTOR3 &normal, VECTOR3 &radial, VECTOR3 &perpendicular);
  bool GetTargetDirections(VECTOR3 &tgtpos, VECTOR3 &tgtvel);
  void CalculateDirection(VECTOR3 euler, VECTOR3 &dir);
  bool GetReferenceName(char *string, int n);

private:
	mutable FLIGHTSTATUS fs;

};

#endif