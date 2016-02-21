#define STRICT
#define ORBITER_MODULE
#include "windows.h"
#include "orbitersdk.h"
#include "AttitudeIndicatorMFD.h"
#include "ADI.h"
#include "AttitudeReferenceADI.h"

// ==============================================================
// Global variables

static struct {
	int mode;
	AttitudeIndicatorMFD *CurrentMFD;
} g_AttitudeIndicatorMFD;


// ==============================================================
// API interface

DLLCLBK void InitModule (HINSTANCE hDLL)
{
	static char *name = "ADI";   // MFD mode name
	MFDMODESPECEX spec;
	spec.name = name;
	spec.key = OAPI_KEY_T;                // MFD mode selection key
	spec.context = NULL;
	spec.msgproc = AttitudeIndicatorMFD::MsgProc;  // MFD mode callback function

	// Register the new MFD mode with Orbiter
	g_AttitudeIndicatorMFD.mode = oapiRegisterMFDMode(spec);
	g_AttitudeIndicatorMFD.CurrentMFD = NULL;
}

DLLCLBK void ExitModule (HINSTANCE hDLL)
{
	// Unregister the custom MFD mode when the module is unloaded
	oapiUnregisterMFDMode(g_AttitudeIndicatorMFD.mode);
}

// ==============================================================
// MFD class implementation

// Constructor
AttitudeIndicatorMFD::AttitudeIndicatorMFD(DWORD w, DWORD h, VESSEL *vessel)
: MFD2 (w, h, vessel)
{
	font = oapiCreateFont (w/20, true, "Arial", FONT_NORMAL, 450);
	// Add MFD initialisation here
	g_AttitudeIndicatorMFD.CurrentMFD = this;
	attref = new AttitudeReferenceADI(pV);
	adi = new ADI(1, 1, w - 2, h * 2 / 3, attref, cw, ch);
	zoom = 2;
	mode = 3;
}

// Destructor
AttitudeIndicatorMFD::~AttitudeIndicatorMFD()
{
	oapiReleaseFont (font);
	// Add MFD cleanup code here
	delete (attref);
	delete (adi);
}

// Return button labels
char *AttitudeIndicatorMFD::ButtonLabel(int bt)
{
	// The labels for the buttons used by our MFD mode
	static char *label[3] = {"FRM", "Z+", "Z-"};
	return (bt < 3 ? label[bt] : 0);
}

// Return button menus
int AttitudeIndicatorMFD::ButtonMenu(const MFDBUTTONMENU **menu) const
{
	// The menu descriptions for the buttons
	static const MFDBUTTONMENU mnu[3] = {
		{ "Change Frame", 0, 'F' },
		{ "Zoom in", 0, 'I' },
		{"Zoom out", 0, 'O'}
	};
	if (menu) *menu = mnu;
	return 3; // return the number of buttons used
}

bool AttitudeIndicatorMFD::ConsumeButton(int bt, int event)
{
	if (!(event & PANEL_MOUSE_LBDOWN)) return false;
	static const DWORD btkey[3] = { OAPI_KEY_F, OAPI_KEY_I, OAPI_KEY_O };
	if (bt < 3) return ConsumeKeyBuffered(btkey[bt]);
	else return false;
}

bool AttitudeIndicatorMFD::ConsumeKeyBuffered(DWORD key)
{
	switch (key) {
	case OAPI_KEY_F:
		mode = (mode + 1) % modeCount;
		attref->SetMode(mode);
		return true;
	case OAPI_KEY_I:
		zoom += 0.5;
		return true;
	case OAPI_KEY_O:
		zoom -= 0.5;
		return true;
	}
	return false;
}


void AttitudeIndicatorMFD::PostStep(double simt, double simdt, double mjd) {
	attref->PostStep(simt, simdt, mjd);
}

// Repaint the MFD
bool AttitudeIndicatorMFD::Update(oapi::Sketchpad *skp)
{
	Title (skp, "Attitude Indicator");
	// Draws the MFD title

	/*
	skp->SetFont (font);
	skp->SetTextAlign (oapi::Sketchpad::CENTER, oapi::Sketchpad::BASELINE);
	skp->SetTextColor (0x00FFFF);
	skp->Text (W/2, H/2,"Display area", 12);
	skp->Rectangle (W/4, H/4, (3*W)/4, (3*H)/4);

	VECTOR3 arot;
	VESSEL *v = oapiGetFocusInterface();
	v->GetGlobalOrientation(arot);
	*/

	// Add MFD display routines here.
	// Use the device context (hDC) for Windows GDI paint functions.

	if (g_AttitudeIndicatorMFD.CurrentMFD != NULL) {
		g_AttitudeIndicatorMFD.CurrentMFD->PostStep(oapiGetSimTime(), oapiGetSimStep(), oapiGetSimMJD());
	}
	adi->DrawBall(skp, zoom);

	skp->SetTextColor(WHITE);
	skp->Text(10, 10, modeStrings[mode], 3);
	return true;
}

// MFD message parser
int AttitudeIndicatorMFD::MsgProc(UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam)
{
	switch (msg) {
	case OAPI_MSG_MFD_OPENED:
		// Our new MFD mode has been selected, so we create the MFD and
		// return a pointer to it.
		return (int)(new AttitudeIndicatorMFD(LOWORD(wparam), HIWORD(wparam), (VESSEL*)lparam));
	}
	return 0;
}

