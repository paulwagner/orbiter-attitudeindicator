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
	penBlue = oapiCreatePen(1, 1, BLUE);
	penGreen = oapiCreatePen(1, 1, GREEN);
	penWhite = oapiCreatePen(1, 1, WHITE);
	penBlack = oapiCreatePen(1, 1, BLACK);
	brushBlue = oapiCreateBrush(BLUE);
	brushGreen = oapiCreateBrush(GREEN);
	brushWhite = oapiCreateBrush(WHITE);
	brushBlack = oapiCreateBrush(BLACK);
	// MFD initialisation
	g_AttitudeIndicatorMFD.CurrentMFD = this;
	attref = new AttitudeReferenceADI(pV);
	config = new Configuration();
	if (!config->loadConfig(CONFIG_FILE)) {
		oapiWriteLog("AttitudeIndicatorMFD::Failed to load config.");
	}
	CreateADI();
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
	delete (penBlue);
	delete (penGreen);
	delete (penWhite);
	delete (penBlack);
	delete (brushBlue);
	delete (brushGreen);
	delete (brushWhite);
	delete (brushBlack);
}

void AttitudeIndicatorMFD::CreateADI() {
	if (adi)
		delete adi;
	switch (mode) {
	case 0:
		adi = new ADI(1, 1, W - 2, H * 2 / 3, attref, 15, 15, config->getConfig());
		break;
	case 1:
	case 2:
		adi = new ADI(1, 1, W - 2, H - 2, attref, 15, 15, config->getConfig());
		break;
	default:
		break;
	}
	adi->SetRateIndicators(mode != 2);
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
		CreateADI();
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

	if (mode == 0)
		DrawDataField(skp, 1, (H * 2 / 3) + 1, W - 2, (H / 3) - 2);

	skp->SetTextColor(WHITE);
	std::ostringstream sstream;
	sstream << "Frm: " << frmStrings[frm];
	//skp->Text(15, 15, sstream.str().c_str(), sstream.str().length());
	char buf[50];
	if (attref->GetReferenceName(buf, 50)) {
		std::string str = "Ref: ";
		str.append(buf);
		//skp->Text(W/2 + 15, 15, str.c_str(), str.length());
	}
	return true;
}

void AttitudeIndicatorMFD::DrawDataField(oapi::Sketchpad *skp, int x, int y, int width, int height) {
	std::string s;
	// Set font

	// Draw text
	skp->SetTextColor(WHITE);
	/*
	skp->Text(x, y, frmStrings[frm], strlen(frmStrings[frm]));
	char buf[20];
	attref->GetReferenceName(buf, 20);
	skp->Text(x + width / 4, y, buf, strlen(buf));
	*/

	// Layout control points
	int cp1_x = x + width / 4;
	int cp1_y = y + 35; // Compass height
	int cp2_x = x + width * 3 / 4;
	int cp2_y = y;

	// Draw compass
	skp->SetPen(penBlue);
	skp->SetBrush(brushBlue);
	skp->Rectangle(cp1_x, cp1_y, cp2_x, cp2_y);
	int degs = 70; // Degrees visible in compass
	int degpx = (int)(width / 2 / (double)degs); // Pixel per degree
	if (degpx <= 0)
		degpx = 1;
	int heading = (int)(attref->GetFlightStatus().heading);
	skp->SetPen(penWhite);
	skp->SetBrush(brushWhite);
	skp->SetTextColor(WHITE);
	int h = (heading - degs/2) % 360;
	for (int tx = (width / 4); tx <= (width * 3 / 4); tx += degpx) {
		if (h % 10 == 0) {
			skp->Line(x + tx, cp1_y, x + tx, cp1_y - 10);
		}
		else if (h % 2 == 0) {
			skp->Line(x + tx, cp1_y, x + tx, cp1_y - 5);
		}
		if (h % 30 == 0) {
			switch (h) {
			case 0:	  {s = "N"; break; }
			case 90:  {s = "E"; break; }
			case 180: {s = "S"; break; }
			case 270: {s = "W"; break; }
			default:  {s = std::to_string(h); break; }
			}
			//s = std::to_string(h);
			int tw = skp->GetTextWidth(s.c_str());
			int th = skp->GetCharSize() & 0xFFFF;
			skp->Text(x + tx - (tw / 2), cp1_y - 15 - th, s.c_str(), s.length());
		}
		h++;
		h %= 360;
	}
	s = std::to_string(heading);
	skp->SetPen(penGreen);
	skp->Line(width / 2, cp1_y, width / 2, cp2_y);
	int tw = skp->GetTextWidth(s.c_str());
	skp->SetPen(penBlack);
	skp->SetBrush(brushBlack);
	skp->Rectangle((width / 2) - (tw / 2), cp2_y, (width / 2) + (tw / 2), cp1_y - 12);
	skp->SetBrush(NULL);
	skp->SetPen(penGreen);
	skp->Rectangle((width / 2) - (tw / 2), cp2_y, (width / 2) + (tw / 2), cp1_y - 12);
	skp->SetTextColor(GREEN);
	skp->TextBox((width / 2) - (tw / 2), cp2_y, (width / 2) + (tw / 2), cp1_y - 12, s.c_str(), s.length());

	skp->SetPen(penBlue);
	skp->SetBrush(brushBlue);
	skp->SetTextColor(GREEN);
	
	// Draw velocity
	skp->Rectangle(cp1_x, cp1_y, x, y + height);
	skp->TextBox(x, y, cp1_x, cp1_y - 5, "OS m/s", 6);

	// Draw altitude
	skp->Rectangle(cp2_x, cp1_y, x + width, y + height);
	skp->TextBox(cp2_x, cp2_y, x + width, cp1_y - 5, "ALT km", 6);

	// Draw text
	
	skp->SetTextColor(WHITE);
	skp->TextBox(cp1_x, cp1_y, width / 2, 2 * cp1_y, frmStrings[frm], strlen(frmStrings[frm]));
	char buf[50];
	if (attref->GetReferenceName(buf, 50)) {
		skp->TextBox(width / 2, cp1_y, cp2_x, 2 * cp1_y, buf, strlen(buf));
	}

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

