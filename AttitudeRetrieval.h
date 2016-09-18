#ifndef _ATTITUDERETRIEVAL_H_
#define _ATTITUDERETRIEVAL_H_

#include "Orbitersdk.h"

namespace AttitudeRetrieval {
	bool isSupported();
	bool getExternalAttitude(VECTOR3& globRot);
}

#endif