#ifndef _ATTITUDEREFERENCEADI_H_
#define _ATTITUDEREFERENCEADI_H_

#include "attref.h"

typedef struct {
	bool docked;
	bool ground;
	double altitude;
	double pitchrate;
	double yawrate;
	double rollrate;
	double heading;
	double pitch;
	double bank;
	double aoa;
	double dns;
	double stp;
	double tas;
	double os;
	double ias;
	double gs;
	double vs;
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
	bool hasNavTarget;
	VECTOR3 navTargetRelPos;
	VECTOR3 navTargetRelVel;
	double navBrg;
	int navCnt;
	double* navCrs;
	DWORD navType;
} FLIGHTSTATUS;

class AttitudeReferenceADI : public AttitudeReference {
public:
  AttitudeReferenceADI(const VESSEL* vessel);
  ~AttitudeReferenceADI();
  inline FLIGHTSTATUS &GetFlightStatus(){ return fs; };
  bool PostStep(double simt, double simdt, double mjd);
  bool GetOrbitalSpeedDirection(VECTOR3 &prograde, VECTOR3 &normal, VECTOR3 &radial, VECTOR3 &perpendicular);
  bool GetAirspeedDirection(VECTOR3 &prograde, VECTOR3 &normal, VECTOR3 &radial, VECTOR3 &perpendicular);
  bool GetTargetDirections(VECTOR3 &tgtpos, VECTOR3 &tgtvel);
  bool GetReferenceName(char *string, int n);

private:
	FLIGHTSTATUS fs;
	void CalculateDirection(VECTOR3 euler, VECTOR3 &dir);

};

#endif