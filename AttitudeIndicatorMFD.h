#ifndef __ATTITUDEINDICATORMFD_H
#define __ATTITUDEINDICATORMFD_H

#include "MFDCore.h"

#define CONFIG_FILE "MFD\\AttitudeIndicatorMFD.cfg"

class ADI;
class AttitudeReferenceADI;
class Configuration;

class AttitudeIndicatorMFD: public MFD2 {
public:
	AttitudeIndicatorMFD(DWORD w, DWORD h, UINT mfd, VESSEL *vessel);
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
	double chw;
	double chw2;
	double chw3;
	void PostStep(double simt, double simdt, double mjd);
	void CreateADI();
	void DrawDataField(oapi::Sketchpad *skp, int x, int y, int width, int height);
private:
	oapi::Pen *penBlue, *penGreen, *penGreen2, *penRed, *penWhite, *penBlack, *penYellow2;
	oapi::Brush *brushBlue, *brushGreen, *brushGreen2, *brushRed, *brushWhite, *brushBlack, *brushYellow2;
	MFDSettings* settings;

	std::string convertAltString(double altitude);
	std::string convertAngleString(double angle);
	void DrawIndicators(oapi::Sketchpad* skp, int x1, int x2, int y1, int y2, double v, bool b = true);
	void inline FillPtchBnkString(char ptch[6], char bnk[6]);

};

#endif // !__ATTITUDEINDICATORMFD_H