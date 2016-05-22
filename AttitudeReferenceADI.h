#ifndef _ATTITUDEREFERENCEADI_H_
#define _ATTITUDEREFERENCEADI_H_

#include "attref.h"

typedef struct {
	bool docked;
	double altitude;
	double pitchrate;
	double yawrate;
	double rollrate;
	double tas;
	double os;
	double ias;
	double gs;
	double apoapsis;
	double periapsis;
	double apoT;
	double periT;
	double t;
	double ecc;
	double inc;
	double lan;
	double lat;
	double lon;
	double r;
	//OBJHANDLE navTarget;
	bool hasNavTarget;
	VECTOR3 navTargetRelPos;
	VECTOR3 navTargetRelVel;
	//double navTargetInc;
	//double navTargetAp;
	//double navTargetPe;
	DWORD navType;
} FLIGHTSTATUS;

class AttitudeReferenceADI : public AttitudeReference {
public:
  AttitudeReferenceADI(const VESSEL* vessel) : AttitudeReference(vessel){};
  FLIGHTSTATUS &GetFlightStatus();
  bool GetOrbitalSpeedDirection(VECTOR3 &prograde, VECTOR3 &normal, VECTOR3 &radial, VECTOR3 &perpendicular);
  bool GetAirspeedDirection(VECTOR3 &prograde, VECTOR3 &normal, VECTOR3 &radial, VECTOR3 &perpendicular);
  bool GetTargetDirections(VECTOR3 &tgtpos, VECTOR3 &tgtvel);
  bool GetReferenceName(char *string, int n);

private:
	FLIGHTSTATUS fs;
	void CalculateDirection(VECTOR3 euler, VECTOR3 &dir);

};

#endif