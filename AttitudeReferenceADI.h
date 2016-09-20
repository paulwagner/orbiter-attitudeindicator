#ifndef _ATTITUDEREFERENCEADI_H_
#define _ATTITUDEREFERENCEADI_H_

#include "MFDCore.h"

#ifndef ORBITER2016
// From beta OrbiterAPI.h
/**
* \brief Returns the input argument normalised to range 0 ... 2 pi
* \param angle input angle [rad]
* \return normalised angle [rad]
*/
inline double posangle(double angle)
{
	double a = fmod(angle, PI2);
	return (a >= 0.0 ? a : a + PI2);
}
#endif

typedef struct {
	bool docked;
	bool ground;
	double altitude;
	double altitudeGround;
	double pitchrate;
	double yawrate;
	double rollrate;
	double heading;
	double pitch;
	double bank;
	double aoa;
	double dns;
	double stp;
	double dnp;
	double tas;
	double os;
	double ias;
	double gs;
	double tasAcc;
	double osAcc;
	double iasAcc;
	double gsAcc;
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
	DWORD navType;
} FLIGHTSTATUS;

class AttitudeReferenceADI {
public:
  AttitudeReferenceADI(const VESSEL* vessel, const MFDSettings *settings);
  ~AttitudeReferenceADI();

  inline const VESSEL *GetVessel() const { return v; }
  inline const MFDSettings* GetSettings() { return s; }
  inline const MATRIX3 &GetFrameRotMatrix() const { return R; }
  inline const VECTOR3 &GetEulerAngles() const { return euler; }
  inline const VECTOR3 &GetTgtEulerAngles() const { return tgteuler; }
  inline const VECTOR3 &GetTgtVelEulerAngles() const { return tgtveleuler; }
  inline FLIGHTSTATUS &GetFlightStatus() { return fs; }

  bool PostStep(double simt, double simdt, double mjd);
  bool GetOrbitalSpeedDirection(VECTOR3 &prograde, VECTOR3 &normal, VECTOR3 &radial, VECTOR3 &perpendicular);
  bool GetAirspeedDirection(VECTOR3 &prograde, VECTOR3 &normal, VECTOR3 &radial, VECTOR3 &perpendicular);
  bool GetTargetDirections(VECTOR3 &tgtpos, VECTOR3 &tgtvel);
  bool GetManeuverDirections(VECTOR3 &man);
  bool GetReferenceName(char *string, int n);

private:
	const VESSEL *v;
	const MFDSettings* s;
	FLIGHTSTATUS fs;
	void CalculateDirection(VECTOR3 euler, VECTOR3 &dir);
	void UpdateFrameRotMatrix();
	void UpdateEulerAngles();
	void UpdateTargetEulerAngles();

	double prevIAS;
	double prevTAS;
	double prevGS;
	double prevOS;
	double prevAlt;
	double prevt;

	mutable MATRIX3 R;
	mutable VECTOR3 euler;
	mutable VECTOR3 tgteuler;
	mutable VECTOR3 tgtveleuler;

	VECTOR3 prevNavPos;
	VECTOR3 prevVPos;
	double prevst;

};

#endif