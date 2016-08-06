#include "Configuration.h"

Configuration::Configuration () {
	// Default settings
	config.progradeColor = PROGRADE;
	config.normalColor = NORMAL;
	config.radialColor = RADIAL;
	config.perpendicularColor = PERPENDICULAR;
	config.targetColor = TARGET;
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

bool Configuration::loadConfig(const char* file) {
	FILEHANDLE fh = oapiOpenFile(file, FILE_IN, CONFIG);
	if (!fh) {
		return false;
	}
	config.texturePath = (char*)malloc(100);
	oapiReadItem_string(fh, S_TEXTUREPATH, config.texturePath);
	int r, g, b;
	oapiReadItem_int(fh, S_PROGRADE_R, r);
	oapiReadItem_int(fh, S_PROGRADE_G, g);
	oapiReadItem_int(fh, S_PROGRADE_B, b);
	config.progradeColor = RGB(r, g, b);
	oapiReadItem_int(fh, S_NORMAL_R, r);
	oapiReadItem_int(fh, S_NORMAL_G, g);
	oapiReadItem_int(fh, S_NORMAL_B, b);
	config.normalColor = RGB(r, g, b);
	oapiReadItem_int(fh, S_RADIAL_R, r);
	oapiReadItem_int(fh, S_RADIAL_G, g);
	oapiReadItem_int(fh, S_RADIAL_B, b);
	config.radialColor = RGB(r, g, b);
	oapiReadItem_int(fh, S_PERPENDICULAR_R, r);
	oapiReadItem_int(fh, S_PERPENDICULAR_G, g);
	oapiReadItem_int(fh, S_PERPENDICULAR_B, b);
	config.perpendicularColor = RGB(r, g, b);
	oapiReadItem_int(fh, S_TARGET_R, r);
	oapiReadItem_int(fh, S_TARGET_G, g);
	oapiReadItem_int(fh, S_TARGET_B, b);
	config.targetColor = RGB(r, g, b);
	oapiReadItem_int(fh, S_WING_R, r);
	oapiReadItem_int(fh, S_WING_G, g);
	oapiReadItem_int(fh, S_WING_B, b);
	config.wingColor = RGB(r, g, b);
	oapiReadItem_int(fh, S_INDICATOR_R, r);
	oapiReadItem_int(fh, S_INDICATOR_G, g);
	oapiReadItem_int(fh, S_INDICATOR_B, b);
	config.indicatorColor = RGB(r, g, b);
	oapiReadItem_int(fh, S_TURNVEC_R, r);
	oapiReadItem_int(fh, S_TURNVEC_G, g);
	oapiReadItem_int(fh, S_TURNVEC_B, b);
	config.turnVecColor = RGB(r, g, b);
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