#define STRICT
#define ORBITER_MODULE
#include "windows.h"
#include "orbitersdk.h"
#include "AttitudeIndicatorMFD.h"
#include "ADI.h"
#include "AttitudeReferenceADI.h"
#include <sstream>
#include "Configuration.h"

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
	// MFD initialisation
	g_AttitudeIndicatorMFD.CurrentMFD = this;
	attref = new AttitudeReferenceADI(pV);
	config = new Configuration();
	if (!config->loadConfig(CONFIG_FILE)) {
		oapiWriteLog("AttitudeIndicatorMFD::Failed to load config.");
	}
	adi = new ADI(1, 1, w - 2, h * 2 / 3, attref, 10, 10, config->getConfig());
	zoom = DEFAULT_ZOOM;
	mode = DEFAULT_MODE;
	frm = DEFAULT_FRAME;
	attref->SetMode(frm);
}

// Destructor
AttitudeIndicatorMFD::~AttitudeIndicatorMFD()
{
	// MFD cleanup code
	delete (attref);
	delete (adi);
	delete (config);
}

// Return button labels
char *AttitudeIndicatorMFD::ButtonLabel(int bt)
{
	// The labels for the buttons used by our MFD mode
	static char *label[8] = {"FRM", "MOD", "PGD", "NRM", "RAD", "TRN", "Z+", "Z-"};
	return (bt < 8 ? label[bt] : 0);
}

// Return button menus
int AttitudeIndicatorMFD::ButtonMenu(const MFDBUTTONMENU **menu) const
{
	// The menu descriptions for the buttons
	static const MFDBUTTONMENU mnu[8] = {
		{ "Change Frame", 0, 'F' },
		{ "Change Mode", 0, 'M' },
		{ "Toggle Prograde/Retrograde", 0, 'P' },
		{ "Toggle Normal/Antinormal", 0, 'N' },
		{ "Toggle Radial/Antiradial", 0, 'R' },
		{ "Toggle Turn Vector", 0, 'T' },
		{ "Zoom in", 0, 'I' },
		{"Zoom out", 0, 'O'}
	};
	if (menu) *menu = mnu;
	return 8; // return the number of buttons used
}

bool AttitudeIndicatorMFD::ConsumeButton(int bt, int event)
{
	if (!(event & PANEL_MOUSE_LBDOWN)) return false;
	static const DWORD btkey[8] = { OAPI_KEY_F, OAPI_KEY_M, OAPI_KEY_P, OAPI_KEY_N, OAPI_KEY_R, OAPI_KEY_T, OAPI_KEY_I, OAPI_KEY_O };
	if (bt < 8) return ConsumeKeyBuffered(btkey[bt]);
	else return false;
}

bool AttitudeIndicatorMFD::ConsumeKeyBuffered(DWORD key)
{
	switch (key) {
	case OAPI_KEY_F:
		frm = (frm + 1) % frmCount;
		attref->SetMode(frm);
		return true;
	case OAPI_KEY_M:
		mode = (mode + 1) % modeCount;
		// TODO: Implement mode
		return true;
	case OAPI_KEY_P:
		adi->TogglePrograde();
		return true;
	case OAPI_KEY_N:
		adi->ToggleNormal();
		return true;
	case OAPI_KEY_R:
		adi->ToggleRadial();
		return true;
	case OAPI_KEY_T:
		adi->ToggleTurnVector();
		return true;
	case OAPI_KEY_I:
		zoom += 0.5;
		return true;
	case OAPI_KEY_O:
		if (zoom > 0.5)
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
	std::ostringstream sstream;
	sstream << "Frame: " << frmStrings[frm];
	skp->Text(10, 10, sstream.str().c_str(), sstream.str().length());
	char buf[50];
	if (attref->GetReferenceName(buf, 50)) {
		std::string str = "Ref: ";
		str.append(buf);
		skp->Text(W/2 + 10, 10, str.c_str(), str.length());
	}
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

