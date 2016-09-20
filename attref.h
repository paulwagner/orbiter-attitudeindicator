// ==============================================================
//                 ORBITER MODULE: ShuttleA
//                  Part of the ORBITER SDK
//          Copyright (C) 2001-2016 Martin Schweiger
//                   All rights reserved
//
// attref.h
// AttitudeReference class interface
// ==============================================================

#ifndef __ATTREF_H
#define __ATTREF_H

#include "Orbitersdk.h"
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

class AttitudeReference {
public:
	AttitudeReference (const VESSEL *vessel, const MFDSettings* settings);
	inline const VESSEL *GetVessel() const { return v; }
	inline const MFDSettings* GetSettings() { return s;  }

	void SetProjMode (int newmode);
	inline int GetProjMode () const { return projmode; }
	// 0=yawrange 360, 1=pitchrange 360

	void SetTgtmode (int newmode);
	inline int GetTgtmode () const { return tgtmode; }
	// 0=no target, 1=fixed, 2=direction of current nav source, 3=rel velocity of current nav source

	const MATRIX3 &GetFrameRotMatrix () const;
	const VECTOR3 &GetEulerAngles () const;
	void SetEulerOffset (const VECTOR3 &ofs);
	inline const VECTOR3 &GetEulerOffset () const { return euler_offs; }
	bool GetTgtEulerAngles (VECTOR3 &tgt_euler) const;
	void SetTgtOffset (const VECTOR3 &ofs);
	inline const VECTOR3 &GetTgtOffset () const { return tgt_offs; }
	void PostStep (double simt, double simdt, double mjd);
	inline void invalidateAttitude() { valid_axes = valid_euler = valid_tgteuler = false; };


private:
	const VESSEL *v;
	const MFDSettings* s;
	int projmode;
	int tgtmode;
	mutable MATRIX3 R;
	mutable VECTOR3 euler;
	mutable VECTOR3 tgteuler;
	VECTOR3 euler_offs;
	VECTOR3 tgt_offs;
	mutable VECTOR3 tgt_rvel;
	mutable VECTOR3 tgt_ppos;
	mutable double  tgt_ptime;
	mutable bool valid_axes;
	mutable bool valid_euler;
	mutable bool valid_tgteuler;
	mutable bool have_tgteuler;
};

#endif // !__ATTREF_H