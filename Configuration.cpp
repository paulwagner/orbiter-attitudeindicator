#include "Configuration.h"

Configuration::Configuration () {
	// Default settings
	config.progradeColor = PROGRADE;
	config.normalColor = NORMAL;
	config.radialColor = RADIAL;
	config.wingColor = WING;
	config.indicatorColor = INDICATOR;
	config.turnVecColor = TURNVECTOR;
	config.texturePath = 0;
}

bool Configuration::loadConfig(const char* file) {
	// TODO load config
	return false;
}

CONFIGURATION &Configuration::getConfig() {
	return config;
}