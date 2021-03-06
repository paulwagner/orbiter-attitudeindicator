#ifndef _MFDCORE_H_
#define _MFDCORE_H_

#include "Orbitersdk.h"
#include "Configuration.h"
#include <vector>

using namespace std;

#define DEFAULT_ZOOM 1.2
#define DEFAULT_SPEED 0;
#define DEFAULT_LHLN_DATA_MODE 0;

#define TURNVECTORCOUNT 3

// 0=ecliptic, 1=equator, 2=orbit, 3=local horizon, 4+ = NAV receiver
const int frmCount = 5;
const std::string frmStrings[5] = { "ECL", "EQU", "OV/OM", "LH/LN", "NAV" };

// 0=normal, 1=big
const int modeCount = 2;

// 0=GS, 1=TAS, 2=IAS
const int speedCount = 3;

// 0=SRF, 1=ORB
const int lhlnDataCount = 2;

typedef struct _MFDSettings {
	OBJHANDLE vessel = 0;
	int mfd = 0;
	float zoom;
	int frm;
	int mode;
	int speedMode;
	int lhlnDataMode;
	bool drawPrograde;
	bool drawNormal;
	bool drawRadial;
	bool drawPerpendicular;
	// 0 - No turn vectors; 1 - PRIs; 2 - Turn vectors on ball
	int turnVectorMode;
	bool hasManRot;
	VECTOR3 manRot;
	bool idsDockRef;
	int navId;
	double* navCrs;
} MFDSettings;

MFDSettings* loadSettings(CONFIGURATION& config, VESSEL* vessel, int mfd);
void freeAllSettings();

#endif