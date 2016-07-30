#ifndef __ATTITUDEINDICATORMFD_H
#define __ATTITUDEINDICATORMFD_H

#define CONFIG_FILE "MFD\\AttitudeIndicatorMFD.cfg"
#define DEFAULT_FRAME 3
#define DEFAULT_ZOOM 1.2
#define DEFAULT_MODE 0
#define DEFAULT_SPEED 0;
#define DEFAULT_LHLN_DATA_MODE 0;

class ADI;
class AttitudeReferenceADI;
class Configuration;

// 0=ecliptic, 1=equator, 2=orbit, 3=local horizon, 4+ = NAV receiver
const int frmCount = 5;
const std::string frmStrings[5] = { "ECL", "EQU", "OV/OM", "LH/LN", "NAV" };

// 0=normal, 2=big
const int modeCount = 2;

// 0=GS, 1=TAS, 2=IAS
const int speedCount = 3;

// 0=SRF, 1=ORB
const int lhlnDataCount = 2;

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
	int chw;
	void PostStep(double simt, double simdt, double mjd);
	void CreateADI();
	void DrawDataField(oapi::Sketchpad *skp, int x, int y, int width, int height);
	float zoom;
	int frm;
	int mode;
	int speedMode;
	int lhlnDataMode;
private:
	oapi::Pen *penBlue, *penGreen, *penGreen2, *penRed, *penWhite, *penBlack, *penYellow2;
	oapi::Brush *brushBlue, *brushGreen, *brushGreen2, *brushRed, *brushWhite, *brushBlack, *brushYellow2;

	std::string convertAltString(double altitude);
	std::string convertAngleString(double angle);
	void DrawIndicators(oapi::Sketchpad* skp, int x1, int x2, int y1, int y2, double v, bool b = true);
	void inline FillPtchBnkString(char ptch[6], char bnk[6]);

};

#endif // !__ATTITUDEINDICATORMFD_H