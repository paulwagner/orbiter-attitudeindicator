#include "AttitudeRetrieval.h"

#ifdef TRANSX_BUILD
#include "EnjoLib\ModuleMessagingExt.hpp"
#endif

namespace AttitudeRetrieval {
	bool isSupported() {
#ifndef TRANSX_BUILD
		return false;
#else
		return true;
#endif
	}

	bool getExternalAttitude(VECTOR3& globRot) {
#ifdef TRANSX_BUILD
		// Get from TransX
		using namespace EnjoLib;
		ModuleMessagingExt mm;
		VECTOR3 targetVel;
		if (mm.ModMsgGet("TransX", "targetVel", &targetVel)) {
			normalise(targetVel);
			double b = asin(-targetVel.x);
			double a = asin(targetVel.y / cos(b));
			globRot = _V(a, b, 0);
			return true;
		}
		return false;
#else
		return false;
#endif
	}
}