#ifndef __ATTITUDEINDICATORMFD_H
#define __ATTITUDEINDICATORMFD_H

#define CONFIG_FILE "MFD\\AttitudeIndicatorMFD.cfg"
#define DEFAULT_FRAME 3
#define DEFAULT_ZOOM 1.5
#define DEFAULT_MODE 0
#define DEFAULT_SPEED 0;

class ADI;
class AttitudeReferenceADI;
class Configuration;

// 0=ecliptic, 1=equator, 2=orbit, 3=local horizon, 4+ = NAV receiver
const int frmCount = 5;
const char* frmStrings[5] = { "ECL", "EQU", "OV/OM", "LH/LN", "NAV" };

// 0=normal, 2=big
const int modeCount = 2;

// 0=TAS, 1=OS
const int speedCount = 2;

class AttitudeIndicatorMFD: public MFD2 {
public:
	AttitudeIndicatorMFD(DWORD w, DWORD h, VESSEL *vessel);
	~AttitudeIndicatorMFD();
	char *ButtonLabel (int bt);
	int ButtonMenu (const MFDBUTTONMENU **menu) const;
	bool ConsumeButton(int bt, int event);
	bool ConsumeKeyBuffered(DWORD key);
	bool Update (oapi::Sketchpad *skp);
	static int MsgProc (UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam);

protected:
	AttitudeReferenceADI *attref;
	Configuration *config;
	ADI *adi;
	void PostStep(double simt, double simdt, double mjd);
	void CreateADI();
	void DrawDataField(oapi::Sketchpad *skp, int x, int y, int width, int height);
	float zoom;
	int frm;
	int mode;
	int speedMode;
private:
	oapi::Pen *penBlue, *penGreen, *penRed, *penWhite, *penBlack;
	oapi::Brush *brushBlue, *brushGreen, *brushRed, *brushWhite, *brushBlack;

	std::string convertAltString(double altitude);

};

#endif // !__ATTITUDEINDICATORMFD_H