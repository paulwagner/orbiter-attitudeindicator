#ifndef __ATTITUDEINDICATORMFD_H
#define __ATTITUDEINDICATORMFD_H

class ADI;
class AttitudeReferenceADI;

// 0=ecliptic, 1=equator, 2=orbit, 3=local horizon, 4+ = NAV receiver
const int modeCount = 5;
const char* modeStrings[5] = { "ECL", "EQU", "ORB", "LOH", "NAV" };

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
	oapi::Font *font;
	AttitudeReferenceADI *attref;
	ADI *adi;
	void PostStep(double simt, double simdt, double mjd);
	float zoom;
	int mode;

};

#endif // !__ATTITUDEINDICATORMFD_H