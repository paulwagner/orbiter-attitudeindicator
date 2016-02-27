#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

#include "orbitersdk.h"

#define WHITE RGB(255,255,255)
#define BLACK RGB(0,0,0)
#define RED RGB(255,0,0)
#define GREEN RGB(0,255,0)
#define BLUE RGB(0,0,255)
#define PROGRADE RGB(221,255,0)
#define NORMAL RGB(235,11,255)
#define RADIAL RGB(9,254,239)
#define WING WHITE
#define INDICATOR WHITE
#define TURNVECTOR WHITE
#define STARTPROGRADE TRUE
#define STARTNORMAL TRUE
#define STARTRADIAL TRUE
#define STARTTURNVEC FALSE

#define S_TEXTUREPATH "texture"
#define S_PROGRADE_R "progradeR"
#define S_PROGRADE_G "progradeG"
#define S_PROGRADE_B "progradeB"
#define S_NORMAL_R "normalR"
#define S_NORMAL_G "normalG"
#define S_NORMAL_B "normalB"
#define S_RADIAL_R "radialR"
#define S_RADIAL_G "radialG"
#define S_RADIAL_B "radialB"
#define S_WING_R "wingR"
#define S_WING_G "wingG"
#define S_WING_B "wingB"
#define S_INDICATOR_R "indicatorR"
#define S_INDICATOR_G "indicatorG"
#define S_INDICATOR_B "indicatorB"
#define S_TURNVEC_R "turnVectorR"
#define S_TURNVEC_G "turnVectorG"
#define S_TURNVEC_B "turnVectorB"
#define S_PROGRADE_START "startPrograde"
#define S_NORMAL_START "startNormal"
#define S_RADIAL_START "startRadial"
#define S_TURNVEC_START "startTurnVector"

typedef struct {
	char *texturePath;
	DWORD progradeColor;
	DWORD normalColor;
	DWORD radialColor;
	DWORD wingColor;
	DWORD indicatorColor;
	DWORD turnVecColor;
	bool startPrograde;
	bool startNormal;
	bool startRadial;
	bool startTurnVector;
} CONFIGURATION;

class Configuration {
public:
	Configuration();
	~Configuration();
  bool loadConfig(const char* file);
  CONFIGURATION &getConfig();
protected:
	CONFIGURATION config;
};

#endif