#include "MFDCore.h"

static vector<MFDSettings*> settingsMap;

MFDSettings* loadSettings(CONFIGURATION& config, VESSEL* vessel, int mfd) {
	MFDSettings* settings;
	bool hasSettings = false;
	for (vector<MFDSettings*>::iterator it = settingsMap.begin(); it != settingsMap.end(); ++it) {
		if ((*it) && (*it)->vessel == vessel->GetHandle() && (*it)->mfd == mfd) {
			hasSettings = true;
			settings = (*it);
			break;
		}
	}
	if (!hasSettings) {
		settings = (MFDSettings*)malloc(sizeof(MFDSettings));
		settings->vessel = vessel->GetHandle();
		settings->mfd = mfd;
		settings->zoom = DEFAULT_ZOOM;
		settings->mode = config.startMode;
		settings->frm = config.startFrame;
		settings->speedMode = DEFAULT_SPEED;
		settings->lhlnDataMode = DEFAULT_LHLN_DATA_MODE;
		settings->drawPrograde = config.startPrograde;
		settings->drawNormal = config.startNormal;
		settings->drawRadial = config.startRadial;
		settings->drawPerpendicular = config.startPerpendicular;
		settings->turnVectorMode = config.startTurnVectorMode;
		settings->hasManRot = false;
		settings->idsDockRef = false;
		settings->navId = 0;
		int navCount = vessel->GetNavCount();
		settings->navCrs = (double*)malloc(sizeof(double) * navCount);
		for (int i = 0; i < navCount; i++)
			settings->navCrs[i] = 0;
		settingsMap.push_back(settings);
	}
	return settings;
}

void freeAllSettings() {
	for (vector<MFDSettings*>::iterator it = settingsMap.begin(); it != settingsMap.end(); ++it) {
		if ((*it)) {
			if ((*it)->navCrs)
				free((*it)->navCrs);
			free((*it));
		}
	}
}