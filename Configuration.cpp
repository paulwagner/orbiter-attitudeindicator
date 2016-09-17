#include "Configuration.h"

Configuration::Configuration () {
	// Default settings
	config.progradeColor = PROGRADE;
	config.normalColor = NORMAL;
	config.radialColor = RADIAL;
	config.perpendicularColor = PERPENDICULAR;
	config.targetColor = TARGET;
	config.maneuverColor = MANEUVER;
	config.wingColor = WING;
	config.indicatorColor = INDICATOR;
	config.turnVecColor = TURNVECTOR;
	config.texturePath = 0;
	config.startPrograde = STARTPROGRADE;
	config.startNormal = STARTNORMAL;
	config.startRadial = STARTRADIAL;
	config.startPerpendicular = STARTPERPENDICULAR;
	config.startTurnVectorMode = STARTTURNVECMODE;
	config.startMode = STARTMODE;
	config.startFrame = STARTFRAME;
}

Configuration::~Configuration() {
	if (config.texturePath)
		free(config.texturePath);
}

bool Configuration::readRGBFromConfig(FILEHANDLE fh, char* rs, char* gs, char* bs, DWORD& c) {
	int r, g, b;
	bool a = true;
	a &= oapiReadItem_int(fh, rs, r);
	a &= oapiReadItem_int(fh, gs, g);
	a &= oapiReadItem_int(fh, bs, b);
	if (a) c = RGB(r, g, b);
	return a;
}

bool Configuration::loadConfig(const char* file) {
	FILEHANDLE fh = oapiOpenFile(file, FILE_IN, CONFIG);
	if (!fh) {
		return false;
	}
	config.texturePath = (char*)malloc(100);
	oapiReadItem_string(fh, S_TEXTUREPATH, config.texturePath);
	readRGBFromConfig(fh, S_PROGRADE_R, S_PROGRADE_G, S_PROGRADE_B, config.progradeColor);
	readRGBFromConfig(fh, S_NORMAL_R, S_NORMAL_G, S_NORMAL_B, config.normalColor);
	readRGBFromConfig(fh, S_RADIAL_R, S_RADIAL_G, S_RADIAL_B, config.radialColor);
	readRGBFromConfig(fh, S_PERPENDICULAR_R, S_PERPENDICULAR_G, S_PERPENDICULAR_B, config.perpendicularColor);
	readRGBFromConfig(fh, S_TARGET_R, S_TARGET_G, S_TARGET_B, config.targetColor);
	readRGBFromConfig(fh, S_MANEUVER_R, S_MANEUVER_G, S_MANEUVER_B, config.maneuverColor);
	readRGBFromConfig(fh, S_WING_R, S_WING_G, S_WING_B, config.wingColor);
	readRGBFromConfig(fh, S_INDICATOR_R, S_INDICATOR_G, S_INDICATOR_B, config.indicatorColor);
	readRGBFromConfig(fh, S_TURNVEC_R, S_TURNVEC_G, S_TURNVEC_B, config.turnVecColor);
	oapiReadItem_bool(fh, S_PROGRADE_START, config.startPrograde);
	oapiReadItem_bool(fh, S_NORMAL_START, config.startNormal);
	oapiReadItem_bool(fh, S_RADIAL_START, config.startRadial);
	oapiReadItem_bool(fh, S_PERPENDICULAR_START, config.startPerpendicular);
	oapiReadItem_int(fh, S_TURNVECMODE_START, config.startTurnVectorMode);
	oapiReadItem_int(fh, S_MODE_START, config.startMode);
	oapiReadItem_int(fh, S_FRAME_START, config.startFrame);
	oapiCloseFile(fh, FILE_IN);
	return true;
}

CONFIGURATION &Configuration::getConfig() {
	return config;
}