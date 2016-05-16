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
	penRed = oapiCreatePen(1, 1, RED);
	penWhite = oapiCreatePen(1, 1, WHITE);
	penBlack = oapiCreatePen(1, 1, BLACK);
	brushBlue = oapiCreateBrush(BLUE);
	brushGreen = oapiCreateBrush(GREEN);
	brushRed = oapiCreateBrush(RED);
	brushWhite = oapiCreateBrush(WHITE);
	brushBlack = oapiCreateBrush(BLACK);
	// MFD initialisation
	g_AttitudeIndicatorMFD.CurrentMFD = this;
	attref = new AttitudeReferenceADI(pV);
	config = new Configuration();
	if (!config->loadConfig(CONFIG_FILE)) {
		oapiWriteLog("AttitudeIndicatorMFD::Failed to load config.");
	}
	zoom = DEFAULT_ZOOM;
	mode = DEFAULT_MODE;
	frm = DEFAULT_FRAME;
	speedMode = DEFAULT_SPEED;
	CreateADI();
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
	delete (penRed);
	delete (penWhite);
	delete (penBlack);
	delete (brushBlue);
	delete (brushGreen);
	delete (brushRed);
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
		adi = new ADI(1, 1, W - 2, H - 2, attref, 15, 15, config->getConfig());
		break;
	default:
		oapiWriteLog("AttitudeIndicatorMFD::ERROR! Invalid mode flag. Defaulting.");
		adi = new ADI(1, 1, W - 2, H - 2, attref, 15, 15, config->getConfig());
		break;
	}
}

// Return button labels
char *AttitudeIndicatorMFD::ButtonLabel(int bt)
{
	// The labels for the buttons used by our MFD mode
	static char *label[10] = {"MOD", "PGD", "NRM", "RAD", "PRI", "TRN", "Z+", "Z-", "FRM", "SPD"};
	return (bt < 10 ? label[bt] : 0);
}

// Return button menus
int AttitudeIndicatorMFD::ButtonMenu(const MFDBUTTONMENU **menu) const
{
	// The menu descriptions for the buttons
	static const MFDBUTTONMENU mnu[10] = {
		{ "Change Mode", 0, 'M' },
		{ "Toggle Prograde/Retrograde", 0, 'P' },
		{ "Toggle Normal/Antinormal", 0, 'N' },
		{ "Toggle Radial/Antiradial", 0, 'R' },
		{ "Toggle Pitch Rate Indicators", 0, 'D' },
		{ "Toggle Turn Vector Indicator", 0, 'T' },
		{ "Zoom in", 0, 'I' },
		{"Zoom out", 0, 'O'},
		{ "Change Frame", 0, 'F' },
		{ "Change Speed", 0, 'S' }
	};
	if (menu) *menu = mnu;
	return 10; // return the number of buttons used
}

bool AttitudeIndicatorMFD::ConsumeButton(int bt, int event)
{
	if (!(event & PANEL_MOUSE_LBDOWN)) return false;
	static const DWORD btkey[10] = { OAPI_KEY_M, OAPI_KEY_P, OAPI_KEY_N, OAPI_KEY_R, OAPI_KEY_D, OAPI_KEY_T, OAPI_KEY_I, OAPI_KEY_O, OAPI_KEY_F, OAPI_KEY_S };
	if (bt < 10) return ConsumeKeyBuffered(btkey[bt]);
	else return false;
}

bool AttitudeIndicatorMFD::ConsumeKeyBuffered(DWORD key)
{
	switch (key) {
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
	case OAPI_KEY_D:
		adi->ToggleRateIndicators();
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
	case OAPI_KEY_F:
		frm = (frm + 1) % frmCount;
		attref->SetMode(frm);
		return true;
	case OAPI_KEY_S:
		speedMode = (speedMode + 1) % speedCount;
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
	else {
		skp->SetTextColor(WHITE);
		skp->SetBrush(brushBlack);
		int th = skp->GetCharSize() & 0xFFFF;
		int slen = strlen(frmStrings[frm]);
		int swidth = skp->GetTextWidth(frmStrings[frm], slen);
		skp->Rectangle(10, 5, 10 + swidth, 5 + th);
		skp->TextBox(10, 5, 10 + swidth, 5 + th, frmStrings[frm], slen);
		char buf[50];
		if (attref->GetReferenceName(buf, 50)) {
			slen = strlen(buf);
			swidth = skp->GetTextWidth(buf, slen);
			skp->Rectangle(W - swidth - 10, 5, W - 10, 5 + th);
			skp->TextBox(W - swidth - 10, 5, W - 10, 5 + th, buf, slen);
		}
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
	int cp1_x = x + (width / 4) - 15;
	int cp1_y = y + 35; // Compass height
	int cp2_x = x + (width * 3 / 4) + 15;
	int cp2_y = y;

	// Draw compass
	skp->SetPen(penBlue);
	skp->SetBrush(brushBlue);
	skp->Rectangle(cp1_x, cp1_y, cp2_x, cp2_y);
	int degs = 70; // Degrees visible in compass
	int degpx = (int)(width / 2 / (double)degs); // Pixel per degree
	if (degpx <= 0)
		degpx = 1;
	int heading = (int)(round(attref->GetFlightStatus().heading));
	skp->SetPen(penWhite);
	skp->SetBrush(brushWhite);
	skp->SetTextColor(WHITE);
	int h = (heading - degs / 2) % 360;
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
	// Draw current heading
	s = std::to_string(heading);
	int n = 3 - s.length();
	if (n < 0) n = 0;
	s.insert(0, n, '0');
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

	int cur_width = skp->GetTextWidth("00000", 5);
	// Draw velocity
	skp->SetPen(penBlue);
	skp->SetBrush(brushBlue);
	skp->SetTextColor(WHITE);
	skp->Rectangle(cp1_x, cp1_y, x, y + height);
	double airspeed = 0;
	std::string spd;
	if (speedMode == 0) {
		airspeed = (double)(attref->GetFlightStatus().tas);
		spd = "TAS m/s";
	}
	else{
		airspeed = (double)(attref->GetFlightStatus().os);
		spd = "OS m/s";
	}
	skp->TextBox(x + 10, y + 7, cp1_x, cp1_y, spd.c_str(), spd.length());
	int scale = 25;
	int rspd = (int)(round(airspeed*10 / (double)scale) * (double)scale);
	int mid_y = cp1_y + (y + height - cp1_y) / 2;
	for (int k = 0; k < 2; k++) {
		int a = rspd - k * scale;
		int ty = mid_y + k;
		bool stop = true;
		do {
			if (a < 0)
				break;
			skp->SetPen(penWhite);
			skp->SetBrush(brushWhite);
			skp->SetTextColor(WHITE);
			if (a % 1000 == 0) {
				skp->Line(cp1_x, ty, cp1_x - 25, ty);
				s = std::to_string((int)(a/10));
				int n = 5 - s.length();
				if (n < 0) n = 0;
				s.insert(0, n, '0');
				int tw = skp->GetTextWidth(s.c_str());
				int th = skp->GetCharSize() & 0xFFFF;
				skp->Text(cp1_x - 35 - tw, ty - (th / 2), s.c_str(), s.length());
			}
			else if (a % 500 == 0) {
				skp->Line(cp1_x, ty, cp1_x - 10, ty);
			}
			else if (a % 250 == 0) {
				skp->Line(cp1_x, ty, cp1_x - 5, ty);
			}
			if (k == 0) {
				a += scale;
				stop = (ty <= cp1_y);
				ty -= 1;
			}
			else {
				a -= scale;
				stop = (ty >= y + height);
				ty += 1;
			}
		} while (!stop);
	}
	// Draw current speed
	if (airspeed < 10) {
		s = std::to_string(round((double)airspeed * (double)100) / (double)100);
		std::string::size_type n = s.find('.');
		s = s.substr(0, n + 3);
	}
	else if (airspeed < 100) {
		s = std::to_string(round((double)airspeed * (double)10) / (double)10);
		std::string::size_type n = s.find('.');
		s = s.substr(0, n + 2);
	}
	else {
		s = std::to_string((int)round(airspeed));
	}
	skp->SetPen(penGreen);
	skp->Line(cp1_x, mid_y, cp1_x - 30, mid_y);
	tw = skp->GetTextWidth(s.c_str());
	int th = skp->GetCharSize() & 0xFFFF;
	skp->SetPen(penBlack);
	skp->SetBrush(brushBlack);
	skp->Rectangle(cp1_x - 35, mid_y - (th / 2), cp1_x - 35 - cur_width, mid_y + (th / 2));
	skp->SetBrush(NULL);
	skp->SetPen(penGreen);
	skp->Rectangle(cp1_x - 35, mid_y - (th / 2), cp1_x - 35 - cur_width, mid_y + (th / 2));
	skp->SetTextColor(GREEN);
	int offset = (cur_width - tw);
	skp->TextBox(cp1_x - 35 - (offset / 2) - tw, mid_y - (th / 2), cp1_x - 35 - (offset / 2), mid_y + (th / 2), s.c_str(), s.length());

	// Draw altitude
	skp->SetPen(penBlue);
	skp->SetBrush(brushBlue);
	skp->SetTextColor(WHITE);
	skp->Rectangle(cp2_x, cp1_y, x + width, y + height);
	skp->TextBox(cp2_x + 15, y + 7, x + width, cp1_y, "ALT km", 6);
	double altitude = (double)(attref->GetFlightStatus().altitude);
	scale = 25;
	int ralt = (int)(round(altitude / (double)scale) * (double)scale);
	int apoapsis = (int)(round(attref->GetFlightStatus().apoapsis / (double)scale) * (double)scale);
	int periapsis = (int)(round(attref->GetFlightStatus().periapsis / (double)scale) * (double)scale);
	int ty_apo = -1, ty_peri = -1;
	mid_y = cp1_y + (y + height - cp1_y) / 2;
	for (int k = 0; k < 2; k++) {
		int a = ralt - k * scale;
		int ty = mid_y + k;
		bool stop = true;
		do {
			if (a < 0)
				break;
			skp->SetPen(penWhite);
			skp->SetBrush(brushWhite);
			skp->SetTextColor(WHITE);
			if (a % 1000 == 0) {
				skp->Line(cp2_x, ty, cp2_x + 25, ty);
				s = std::to_string(a / 1000);
				int n = 5 - s.length();
				if (n < 0) n = 0;
				s.insert(0, n, '0');
				int tw = skp->GetTextWidth(s.c_str());
				int th = skp->GetCharSize() & 0xFFFF;
				skp->Text(cp2_x + 35, ty - (th / 2), s.c_str(), s.length());
			}
			else if (a % 500 == 0) {
				skp->Line(cp2_x, ty, cp2_x + 10, ty);
			}
			else if (a % 250 == 0) {
				skp->Line(cp2_x, ty, cp2_x + 5, ty);
			}
			// Remember apoapsis and periapsis
			if (a == apoapsis)
				ty_apo = ty;
			if (a == periapsis)
				ty_peri = ty;
			if (k == 0) {
				a += scale;
				stop = (ty <= cp1_y);
				ty -= 1;
			}
			else {
				a -= scale;
				stop = (ty >= y + height);
				ty += 1;
			}
		} while (!stop);
	}
	// Draw apoapsis and periapsis
	if (ty_peri >= 0) {
		s = "PE";
		skp->SetPen(penRed);
		skp->SetBrush(brushRed);
		skp->Line(cp2_x, ty_peri, cp2_x + 15, ty_peri);
		tw = skp->GetTextWidth(s.c_str());
		int th = skp->GetCharSize() & 0xFFFF;
		skp->Rectangle(cp2_x + 20, ty_peri - (th / 2), cp2_x + 20 + tw, ty_peri + (th / 2));
		skp->SetTextColor(WHITE);
		skp->TextBox(cp2_x + 20, ty_peri - (th / 2), cp2_x + 20 + tw, ty_peri + (th / 2), s.c_str(), s.length());
	}
	if (ty_apo >= 0) {
		s = "AP";
		skp->SetPen(penRed);
		skp->SetBrush(brushRed);
		skp->Line(cp2_x, ty_apo, cp2_x + 15, ty_apo);
		tw = skp->GetTextWidth(s.c_str());
		int th = skp->GetCharSize() & 0xFFFF;
		skp->Rectangle(cp2_x + 20, ty_apo - (th / 2), cp2_x + 20 + tw, ty_apo + (th / 2));
		skp->SetTextColor(WHITE);
		skp->TextBox(cp2_x + 20, ty_apo - (th / 2), cp2_x + 20 + tw, ty_apo + (th / 2), s.c_str(), s.length());
	}
	// Draw current altitude
	if (altitude < 100000) {
		s = std::to_string(round((double)altitude / (double)10) / (double)100);
		std::string::size_type n = s.find('.');
		s = s.substr(0, n + 3);
	}
	else {
		s = std::to_string(round((double)altitude / (double)100) / (double)10);
		std::string::size_type n = s.find('.');
		s = s.substr(0, n + 2);
	}
	skp->SetPen(penGreen);
	skp->Line(cp2_x, mid_y, cp2_x + 30, mid_y);
	tw = skp->GetTextWidth(s.c_str());
	th = skp->GetCharSize() & 0xFFFF;
	skp->SetPen(penBlack);
	skp->SetBrush(brushBlack);
	skp->Rectangle(cp2_x + 35, mid_y - (th / 2), cp2_x + 35 + cur_width, mid_y + (th / 2));
	skp->SetBrush(NULL);
	skp->SetPen(penGreen);
	skp->Rectangle(cp2_x + 35, mid_y - (th / 2), cp2_x + 35 + cur_width, mid_y + (th / 2));
	skp->SetTextColor(GREEN);
	offset = (cur_width - tw);
	skp->TextBox(cp2_x + 35 + (offset / 2), mid_y - (th / 2), cp2_x + 35 + (offset / 2) + tw, mid_y + (th / 2), s.c_str(), s.length());

	// Draw text
	int mid_width = cp2_x - cp1_x;
	skp->SetPen(penGreen);
	skp->Line(cp1_x + 5, cp1_y + 5 + th, cp2_x - 5, cp1_y + 5 + th);
	skp->Line(cp1_x + (mid_width / 2), cp1_y + 5 + th, cp1_x + (mid_width / 2), y + height);
	skp->SetTextColor(WHITE);
	skp->TextBox(cp1_x + 5, cp1_y + 5, cp1_x + (mid_width / 2), cp1_y + 5 + th, frmStrings[frm], strlen(frmStrings[frm]));
	char buf[50];
	if (attref->GetReferenceName(buf, 50)) {
		skp->TextBox(cp1_x + (mid_width / 2) + 5, cp1_y + 5, cp2_x - 5, cp1_y + 5 + th, buf, strlen(buf));
	}
	// Row 1
	int iy = cp1_y + 5 + th;
	s = "ApA";
	s.append(convertAltString(attref->GetFlightStatus().apoapsis));
	skp->TextBox(cp1_x + 5, iy + 5, cp1_x + (mid_width / 2), iy + 5 + th, s.c_str(), s.length());
	s = "PeA";
	s.append(convertAltString(attref->GetFlightStatus().periapsis));
	skp->TextBox(cp1_x + (mid_width / 2) + 5, iy + 5, cp2_x, iy + 5 + th, s.c_str(), s.length());

	// Row 2
	iy += (5 + th);
	s = "ApT";
	s.append(convertAltString(attref->GetFlightStatus().apoT));
	skp->TextBox(cp1_x + 5, iy + 5, cp1_x + (mid_width / 2), iy + 5 + th, s.c_str(), s.length());
	s = "PeT";
	s.append(convertAltString(attref->GetFlightStatus().periT));
	skp->TextBox(cp1_x + (mid_width / 2) + 5, iy + 5, cp2_x, iy + 5 + th, s.c_str(), s.length());

	// Row 3
	iy += (5 + th);
	s = "Ecc ";
	s.append(std::to_string(round(attref->GetFlightStatus().ecc * (double)10000) / (double)10000));
	s = s.substr(0, 10);
	skp->TextBox(cp1_x + 5, iy + 5, cp1_x + (mid_width / 2), iy + 5 + th, s.c_str(), s.length());
	s = "Inc ";
	double incl = attref->GetFlightStatus().inc*DEG;
	double incl_scale = 10;
	if (incl < 100) incl_scale = 100;
	if (incl < 10) incl_scale = 1000;
	s.append(std::to_string(round(incl * incl_scale) / incl_scale));
	s = s.substr(0, 9);
	s.append("°");
	skp->TextBox(cp1_x + (mid_width / 2) + 5, iy + 5, cp2_x, iy + 5 + th, s.c_str(), s.length());
}

std::string AttitudeIndicatorMFD::convertAltString(double altitude) {
	std::string s_suff = "";
	if (abs(altitude) > 1000) {
		s_suff = "k";
		altitude /= 1000; // in k
	}
	if (abs(altitude) > 1000) {
		s_suff = "M";
		altitude /= 1000; // in M
	}
	double scale = 1;
	if (abs(altitude) < 1000) scale = 10;
	if (abs(altitude) < 100) scale = 100;
	if (abs(altitude) < 10) scale = 1000;
	std::string s;
	if (altitude > 0) s = " ";
	s.append(std::to_string(round(altitude * scale) / scale));
	s = s.substr(0, 6);
	s.append(s_suff);
	return s;
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

