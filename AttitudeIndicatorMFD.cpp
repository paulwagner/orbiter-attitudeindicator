#define STRICT
#define ORBITER_MODULE
#include "windows.h"
#include "orbitersdk.h"
#include "AttitudeIndicatorMFD.h"
#include "ADI.h"
#include "AttitudeReferenceADI.h"
#include <sstream>
#include "Configuration.h"

#define SRFNAVTYPE(x) (x == TRANSMITTER_ILS || x == TRANSMITTER_VOR || x == TRANSMITTER_VTOL)

// ==============================================================
// Global variables
static int MFDMode;
static AttitudeIndicatorMFD* CurrentMFD = 0;

// ==============================================================
// API interface

DLLCLBK void InitModule (HINSTANCE hDLL)
{
	oapiWriteLog("[AttitudeIndicatorMFD] Enter: InitModule");
	static char *name = "ADI";   // MFD mode name
	MFDMODESPECEX spec;
	spec.name = name;
	spec.key = OAPI_KEY_T;                // MFD mode selection key
	spec.context = NULL;
	spec.msgproc = AttitudeIndicatorMFD::MsgProc;  // MFD mode callback function

	// Register the new MFD mode with Orbiter
	MFDMode = oapiRegisterMFDMode(spec);
}

DLLCLBK void ExitModule (HINSTANCE hDLL)
{
	oapiWriteLog("[AttitudeIndicatorMFD] Enter: ExitModule");
	// Unregister the custom MFD mode when the module is unloaded
	oapiUnregisterMFDMode(MFDMode);
}

// ==============================================================
// MFD class implementation

// Constructor
AttitudeIndicatorMFD::AttitudeIndicatorMFD(DWORD w, DWORD h, VESSEL *vessel)
: MFD2 (w, h, vessel)
{
	oapiWriteLog("[AttitudeIndicatorMFD] Enter: _cdecl");
	penBlue = oapiCreatePen(1, 1, BLUE);
	penGreen = oapiCreatePen(1, 1, GREEN);
	penGreen2 = oapiCreatePen(1, 1, GREEN2);
	penRed = oapiCreatePen(1, 1, RED);
	penWhite = oapiCreatePen(1, 1, WHITE);
	penBlack = oapiCreatePen(1, 1, BLACK);
	penYellow2 = oapiCreatePen(1, 1, YELLOW2);
	brushBlue = oapiCreateBrush(BLUE);
	brushGreen = oapiCreateBrush(GREEN);
	brushGreen2 = oapiCreateBrush(GREEN2);
	brushRed = oapiCreateBrush(RED);
	brushWhite = oapiCreateBrush(WHITE);
	brushBlack = oapiCreateBrush(BLACK);
	brushYellow2 = oapiCreateBrush(YELLOW2);
	// MFD initialisation
	attref = new AttitudeReferenceADI(pV);
	config = new Configuration();
	if (!config->loadConfig(CONFIG_FILE)) {
		oapiWriteLog("AttitudeIndicatorMFD::Failed to load config.");
	}
	zoom = DEFAULT_ZOOM;
	mode = DEFAULT_MODE;
	frm = DEFAULT_FRAME;
	speedMode = DEFAULT_SPEED;
	chw = (int)round((double)H / 20);
	chw = min(chw,(int)round((double)W / 20));
	adi = 0;
	CreateADI();
	oapiWriteLog("[AttitudeIndicatorMFD] Leave: _cdecl");
}

// Destructor
AttitudeIndicatorMFD::~AttitudeIndicatorMFD()
{
	oapiWriteLog("[AttitudeIndicatorMFD] Enter: _ddecl");
	// MFD cleanup code
	delete (attref);
	delete (adi);
	delete (config);
	delete (penBlue);
	delete (penGreen);
	delete (penGreen2);
	delete (penRed);
	delete (penWhite);
	delete (penBlack);
	delete (penYellow2);
	delete (brushBlue);
	delete (brushGreen);
	delete (brushGreen2);
	delete (brushRed);
	delete (brushWhite);
	delete (brushBlack);
	delete (brushYellow2);
	CurrentMFD = 0;
}

void AttitudeIndicatorMFD::CreateADI() {
	oapiWriteLog("[AttitudeIndicatorMFD] Enter: CreateADI");
	bool saved = false, pgd = false, nrm = false, rad = false, ratei = false;
	int turnv = 0;
	if (adi) {
		saved = true;
		pgd = adi->GetPrograde();
		nrm = adi->GetNormal();
		rad = adi->GetRadial();
		turnv = adi->GetTurnVector();
		delete adi;
	}
	switch (mode) {
	case 0:
		adi = new ADI(1, 1, W - 2, H * 2 / 3, attref, (int)(chw * 2 / 3), (int)(chw * 2 / 3), config->getConfig());
		break;
	case 1:
		adi = new ADI(1, 1, W - 2, H - 2, attref, (int)(chw * 2 / 3), (int)(chw * 2 / 3), config->getConfig());
		break;
	default:
		oapiWriteLog("AttitudeIndicatorMFD::ERROR! Invalid mode flag. Defaulting.");
		adi = new ADI(1, 1, W - 2, H - 2, attref, (int)(chw * 2 / 3), (int)(chw * 2 / 3), config->getConfig());
		break;
	}
	if (saved) {
		adi->SetPrograde(pgd);
		adi->SetNormal(nrm);
		adi->SetRadial(rad);
		adi->SetTurnVector(turnv);
	}
	attref->SetMode(frm);
}

// Return button labels
char *AttitudeIndicatorMFD::ButtonLabel(int bt)
{
	// The labels for the buttons used by our MFD mode
	static char *label[12] = { "FRM", "MOD", "TRI", "", "Z+", "Z-", "PGD", "NML", "PER", "RAD", "SPD", "NAV" };
	if (frm <= 1 && (bt >= 6 && bt < 10)) return ""; // No markers in ECL and EQU
	if (frm == 4 && (bt >= 7 && bt < 10)) return ""; // Only prograde/retrograde in NAV
	if ((mode == 1 || !(frm == 3 || (frm == 4 && SRFNAVTYPE(attref->GetFlightStatus().navType)))) && (bt == 10)) return ""; // SPD only in surface text mode
	if (frm != 4 && (bt == 11)) return ""; // NAV only in NAV mode
	return (bt < 12 ? label[bt] : 0);
}

// Return button menus
int AttitudeIndicatorMFD::ButtonMenu(const MFDBUTTONMENU **menu) const
{
	// The menu descriptions for the buttons
	static const MFDBUTTONMENU mnu[12] = {
		{ "Change Frame", 0, 'F' },
		{ "Change Mode", 0, 'M' },
		{ "Toggle Rate", "Indicators", 'T' },
		{ 0, 0, 0}, // Reserved
		{ "Zoom in", 0, 'I' },
		{ "Zoom out", 0, 'O' },
		{ "Toggle Prograde/", "Retrograde", 'P' },
		{ "Toggle Normal/", "Antinormal", 'N' },
		{ "Toggle", "Perpendicular", 'D' },
		{ "Toggle Radial", 0, 'R' },
		{ "Change Speed", 0, 'S' },
		{ "Select NAV", "Receiver", 'C' }
	};
	if (menu) *menu = mnu;
	return 12; // return the number of buttons used
}

bool AttitudeIndicatorMFD::ConsumeButton(int bt, int event)
{
	if (!(event & PANEL_MOUSE_LBDOWN)) return false;
	static const DWORD btkey[12] = { OAPI_KEY_F, OAPI_KEY_M, OAPI_KEY_T, 0, OAPI_KEY_I, OAPI_KEY_O, OAPI_KEY_P, OAPI_KEY_N, OAPI_KEY_D, OAPI_KEY_R, OAPI_KEY_S, OAPI_KEY_C };
	if (frm <= 1 && (bt >= 6 && bt < 10)) return 0; // No markers in ECL and EQU
	if (frm == 4 && (bt >= 7 && bt < 10)) return 0; // Only prograde/retrograde in NAV
	if ((mode == 1 || !(frm == 3 || (frm == 4 && SRFNAVTYPE(attref->GetFlightStatus().navType)))) && (bt == 10)) return ""; // SPD only in surface text mode
	if (frm != 4 && (bt == 11)) return 0; // NAV only in NAV mode
	if (bt < 12) return ConsumeKeyBuffered(btkey[bt]);
	else return false;
}

bool AttitudeIndicatorMFD::ConsumeKeyBuffered(DWORD key)
{
	switch (key) {
	case OAPI_KEY_M:
		mode = (mode + 1) % modeCount;
		if (mode == 0) zoom += 0.4;
		if (mode == 1) zoom -= 0.4;
		CreateADI();
		InvalidateButtons();
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
		adi->TogglePerpendicular();
		return true;
	case OAPI_KEY_T:
		adi->ToggleTurnVector();
		return true;
	case OAPI_KEY_I:
		zoom += 0.1;
		return true;
	case OAPI_KEY_O:
		if (zoom > 0.2)
			zoom -= 0.1;
		return true;
	case OAPI_KEY_F:
		frm = (frm + 1) % frmCount;
		attref->SetMode(frm);
		InvalidateButtons();
		return true;
	case OAPI_KEY_S:
		speedMode = (speedMode + 1) % speedCount;
		return true;
	case OAPI_KEY_C:
		int nc = attref->GetVessel()->GetNavCount();
		if (nc > 0) {
			int nid = attref->GetNavid();
			attref->SetNavid((nid + 1) % nc);
		}
		return true;
	}
	return false;
}


void AttitudeIndicatorMFD::PostStep(double simt, double simdt, double mjd) {
	attref->PostStep(simt, simdt, mjd);
}

// Repaint the MFD
bool AttitudeIndicatorMFD::Update(oapi::Sketchpad *skp) {
	Title (skp, "Attitude Indicator");
	PostStep(oapiGetSimTime(), oapiGetSimStep(), oapiGetSimMJD());
	adi->DrawBall(skp, zoom);

	if (mode == 0)
		DrawDataField(skp, 1, (H * 2 / 3) + 1, W - 2, (H / 3) - 2);
	else {
		skp->SetTextColor(WHITE);
		skp->SetBrush(brushBlack);
		int th = skp->GetCharSize() & 0xFFFF;
		std::string frmS = frmStrings[frm];
		if (frm == 4) frmS.append(std::to_string(attref->GetNavid() + 1));
		int slen = frmS.length();
		int swidth = skp->GetTextWidth(frmS.c_str(), slen);
		skp->Rectangle(10, 5, 10 + swidth, 5 + th);
		skp->TextBox(10, 5, 10 + swidth, 5 + th, frmS.c_str(), slen);
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

inline int GetScale(int c) {
	if (c < 3) return 1;
	if (c < 8) return 5;
	if (c < 17) return 10;
	if (c < 38) return 25;
	if (c < 83) return 50;
	if (c < 187) return 125;
	return 250;
}

void WriteText(oapi::Sketchpad *skp, int x1, int y1, int kw, std::string k, std::string v) {
	skp->Text(x1, y1, k.c_str(), k.length());
	skp->Text(x1 + kw, y1, v.c_str(), v.length());
}

void AttitudeIndicatorMFD::DrawDataField(oapi::Sketchpad *skp, int x, int y, int width, int height) {
	std::string s;
	FLIGHTSTATUS fs = attref->GetFlightStatus();
	// Set font
	oapi::Font *f = oapiCreateFont(chw, true, "Sans");
	oapi::Font *fsmall = oapiCreateFont(chw - 2, true, "Sans");
	oapi::Font *fxsmall = oapiCreateFont(chw - 4, true, "Sans");
	//oapi::Font *fxxsmall = oapiCreateFont(chw - 9, true, "Sans");
	skp->SetFont(f);
	skp->SetTextColor(WHITE);

	// Adjust borders
	x += 3; width -= 6; height -= 3;

	// Layout control points
	int cp1_x = x + (width / 4) - (int)(chw);
	int cp1_y = y + (int)(1.5*chw); // Top row height
	int cp2_x = x + (width * 3 / 4) + (int)(chw);
	int cp2_y = y;

	int scale = GetScale((int)round(2500 / (double)(y + height - cp1_y)));
	int cur_width = skp->GetTextWidth("00000", 5);
	if (frm != 4 || SRFNAVTYPE(fs.navType)) {
		// Draw velocity
		skp->SetPen(penBlue);
		skp->SetBrush(brushBlue);
		skp->SetTextColor(WHITE);
		skp->Rectangle(cp1_x, cp1_y, x, y + height);
		double airspeed = 0;
		std::string spd;
		if (frm == 3 || (frm == 4 && SRFNAVTYPE(fs.navType))) {
			switch (speedMode) {
			case 0:
				airspeed = fs.gs; spd = "GS m/s"; break;
			case 1:
				airspeed = fs.tas; spd = "TAS m/s"; break;
			case 2:
				airspeed = fs.ias; spd = "IAS m/s"; break;
			}
		} else {
			airspeed = fs.os; spd = "OS m/s";
		}
		int rspd = (int)(round(airspeed * 10 / (double)scale) * (double)scale);
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
					skp->Line(cp1_x, ty, cp1_x - chw, ty);
					s = std::to_string((int)(a / 10));
					int n = 5 - s.length();
					if (n < 0) n = 0;
					s.insert(0, n, '0');
					int tw = skp->GetTextWidth(s.c_str());
					int th = skp->GetCharSize() & 0xFFFF;
					skp->Text(cp1_x - (int)(1.25*chw) - tw, ty - (th / 2), s.c_str(), s.length());
				}
				else if (a % 500 == 0) {
					skp->Line(cp1_x, ty, cp1_x - (int)(chw / 2), ty);
				}
				else if (a % 250 == 0) {
					skp->Line(cp1_x, ty, cp1_x - (int)(chw / 3), ty);
				}
				if (k == 0) {
					a += scale;
					stop = (ty <= cp1_y - (int)(chw / 4));
					ty -= 1;
				}
				else {
					a -= scale;
					stop = (ty >= y + height - (int)(chw / 4));
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
		skp->Line(cp1_x, mid_y, cp1_x - (int)(1.25*chw), mid_y);
		int tw = skp->GetTextWidth(s.c_str());
		int th = skp->GetCharSize() & 0xFFFF;
		skp->SetPen(penBlack);
		skp->SetBrush(brushBlack);
		skp->Rectangle(cp1_x - (int)(1.25*chw), mid_y - (th * 2 / 3), cp1_x - (int)(1.25*chw) - max(cur_width, tw), mid_y + (th * 2 / 3));
		skp->SetBrush(NULL);
		skp->SetPen(penGreen);
		skp->Rectangle(cp1_x - (int)(1.25*chw), mid_y - (th * 2 / 3), cp1_x - (int)(1.25*chw) - max(cur_width, tw), mid_y + (th * 2 / 3));
		skp->SetTextColor(GREEN);
		int offset = (cur_width - tw);
		if (offset < 0)
			offset = 0;
		skp->TextBox(cp1_x - (int)(1.25*chw) - (offset / 2) - tw, mid_y - (th / 2), cp1_x - (int)(1.25*chw) - (offset / 2), mid_y + (th / 2), s.c_str(), s.length());

		// Draw altitude
		skp->SetPen(penBlue);
		skp->SetBrush(brushBlue);
		skp->SetFont(f);
		skp->SetTextColor(WHITE);
		skp->Rectangle(cp2_x, cp1_y, x + width, y + height);
		double altitude = fs.altitude;
		int alt_off = (int)(altitude / (100 * 1000 * 1000));  // in 100k km
		INT64 ralt = (INT64)(round(altitude / (double)scale) * (double)scale);
		INT64 apoapsis = (INT64)(round(fs.apoapsis / (double)scale) * (double)scale);
		INT64 periapsis = (INT64)(round(fs.periapsis / (double)scale) * (double)scale);
		int ty_apo = -1, ty_peri = -1;
		mid_y = cp1_y + (y + height - cp1_y) / 2;
		for (int k = 0; k < 2; k++) {
			INT64 a = ralt - k * scale;
			int ty = mid_y + k;
			bool stop = true;
			do {
				if (a < 0)
					break;
				skp->SetPen(penWhite);
				skp->SetBrush(brushWhite);
				skp->SetTextColor(WHITE);
				if (a % 1000 == 0) {
					skp->Line(cp2_x, ty, cp2_x + chw, ty);
					s = std::to_string((a / 1000) - (INT64)alt_off * (100 * 1000));
					int n = 5 - s.length();
					if (n < 0) n = 0;
					s.insert(0, n, '0');
					int th = skp->GetCharSize() & 0xFFFF;
					skp->Text(cp2_x + (int)(1.25*chw), ty - (th / 2), s.c_str(), s.length());
				}
				else if (a % 500 == 0) {
					skp->Line(cp2_x, ty, cp2_x + (int)(chw / 2), ty);
				}
				else if (a % 250 == 0) {
					skp->Line(cp2_x, ty, cp2_x + (int)(chw / 3), ty);
				}
				// Remember apoapsis and periapsis
				if (a == apoapsis)
					ty_apo = ty;
				if (a == periapsis)
					ty_peri = ty;
				if (k == 0) {
					a += scale;
					stop = (ty <= cp1_y - (int)(chw / 4));
					ty -= 1;
				}
				else {
					a -= scale;
					stop = (ty >= y + height - (int)(chw / 4));
					ty += 1;
				}
			} while (!stop);
		}
		// Draw apoapsis and periapsis
		skp->SetPen(penRed);
		skp->SetBrush(brushRed);
		skp->SetTextColor(WHITE);
		th = skp->GetCharSize() & 0xFFFF;
		if (ty_peri >= 0) {
			s = "PE";
			skp->Line(cp2_x, ty_peri, cp2_x + chw, ty_peri);
			tw = skp->GetTextWidth(s.c_str());
			skp->Rectangle(cp2_x + chw, ty_peri - (th / 2), cp2_x + chw + tw, ty_peri + (th / 2));
			skp->TextBox(cp2_x + chw, ty_peri - (th / 2), cp2_x + chw + tw, ty_peri + (th / 2), s.c_str(), s.length());
		}
		if (ty_apo >= 0) {
			s = "AP";
			skp->Line(cp2_x, ty_apo, cp2_x + chw, ty_apo);
			tw = skp->GetTextWidth(s.c_str());
			skp->Rectangle(cp2_x + chw, ty_apo - (th / 2), cp2_x + chw + tw, ty_apo + (th / 2));
			skp->TextBox(cp2_x + chw, ty_apo - (th / 2), cp2_x + chw + tw, ty_apo + (th / 2), s.c_str(), s.length());
		}
		// Draw current altitude
		double s_altitude = altitude - ((INT64)alt_off * 100 * 1000 * 1000);
		if (s_altitude < 100 * 1000) {
			s = std::to_string(round((double)s_altitude / (double)10) / (double)100);
			std::string::size_type n = s.find('.');
			s = s.substr(0, n + 3);
		}
		else if (s_altitude < 1000 * 1000){
			s = std::to_string(round((double)s_altitude / (double)100) / (double)10);
			std::string::size_type n = s.find('.');
			s = s.substr(0, n + 2);
		}
		else {
			s = std::to_string((int)round((double)s_altitude / (double)1000));
		}
		skp->SetPen(penGreen);
		skp->Line(cp2_x, mid_y, cp2_x + (int)(1.25*chw), mid_y);
		tw = skp->GetTextWidth(s.c_str());
		th = skp->GetCharSize() & 0xFFFF;
		skp->SetPen(penBlack);
		skp->SetBrush(brushBlack);
		skp->Rectangle(cp2_x + (int)(1.25*chw), mid_y - (th * 2 / 3), cp2_x + (int)(1.25*chw) + cur_width, mid_y + (th * 2 / 3));
		skp->SetBrush(NULL);
		skp->SetPen(penGreen);
		skp->Rectangle(cp2_x + (int)(1.25*chw), mid_y - (th * 2 / 3), cp2_x + (int)(1.25*chw) + cur_width, mid_y + (th * 2 / 3));
		skp->SetTextColor(GREEN);
		offset = (cur_width - tw);
		skp->TextBox(cp2_x + (int)(1.25*chw) + (offset / 2), mid_y - (th / 2), cp2_x + (int)(1.25*chw) + (offset / 2) + tw, mid_y + (th / 2), s.c_str(), s.length());

		// Draw top and bottom box
		skp->SetFont(fxsmall);
		th = skp->GetCharSize() & 0xFFFF;
		skp->SetBrush(brushBlack);
		skp->SetPen(penBlack);
		skp->Rectangle(x, y, x + width, cp1_y);
		skp->Rectangle(x, y + height - th, x + width, y + height + 4);
		skp->SetBrush(NULL);
		if (alt_off > 0) {
			s = "+";
			s.append(std::to_string((double)alt_off / 10));
			s = s.substr(0, s.find(".") + 2);
			s.append(" Mkm");
			tw = skp->GetTextWidth(s.c_str(), s.length());
			skp->Text(x + width - tw, y + height - th, s.c_str(), s.length());
		}

		// Draw text strings
		skp->SetFont(fsmall);
		th = skp->GetCharSize() & 0xFFFF;
		double h = cp1_y - y;
		double w = cp1_x - x - skp->GetTextWidth(spd.c_str(), spd.length());
		skp->Text(x + (int)(w / 2), y + (int)(th / 2), spd.c_str(), spd.length());
		std::string alt = "ALT km";
		w = width - cp2_x - skp->GetTextWidth(alt.c_str(), alt.length());
		skp->Text(cp2_x + (int)(w / 2), y + (int)(th / 2), alt.c_str(), alt.length());
	}

	// Draw text
	skp->SetFont(fsmall);
	int chw3 = (int)(chw / 3);
	int kw = (int)(2*chw);
	int mid_width = cp2_x - cp1_x;
	skp->SetPen(penGreen);
	skp->Line(cp1_x + chw3, cp1_y, cp2_x - chw3, cp1_y);
	skp->Line(cp1_x + (mid_width / 2), cp1_y, cp1_x + (mid_width / 2), y + height);
	skp->SetTextColor(WHITE);
	std::string frmS = frmStrings[frm];
	if (frm == 4) frmS.append(std::to_string(attref->GetNavid() + 1));
	skp->TextBox(cp1_x + chw3, y + (int)(chw / 4), cp1_x + (mid_width / 2), cp1_y, frmS.c_str(), frmS.length());
	char buf[50];
	if (attref->GetReferenceName(buf, 50)) {
		skp->TextBox(cp1_x + (mid_width / 2) + chw3, y + (int)(chw / 4), cp2_x - chw3, cp1_y, buf, strlen(buf));
	}
	skp->SetFont(fxsmall);
	int th = skp->GetCharSize() & 0xFFFF;
	int iy = cp1_y + chw3;
	if (frm == 4) {
		if (fs.hasNavTarget) {
			if (SRFNAVTYPE(fs.navType)) {

			}
			else {
				// Row 1
				WriteText(skp, cp1_x + chw3, iy, kw, "DST", convertAltString(length(fs.navTargetRelPos)));
				WriteText(skp, cp1_x + (mid_width / 2) + chw3, iy, kw, "CVEL", convertAltString(-length(fs.navTargetRelVel)));

				// Row 2
				iy += (chw3 + th);
				WriteText(skp, cp1_x + chw3, iy, kw, "XDST", convertAltString(fs.navTargetRelPos.x));
				WriteText(skp, cp1_x + (mid_width / 2) + chw3, iy, kw, "XVEL", convertAltString(fs.navTargetRelVel.x));

				// Row 3
				iy += (chw3 + th);
				WriteText(skp, cp1_x + chw3, iy, kw, "YDST", convertAltString(fs.navTargetRelPos.y));
				WriteText(skp, cp1_x + (mid_width / 2) + chw3, iy, kw, "YVEL", convertAltString(fs.navTargetRelVel.y));

				// Row 4
				iy += (chw3 + th);
				WriteText(skp, cp1_x + chw3, iy, kw, "ZDST", convertAltString(fs.navTargetRelPos.z));
				WriteText(skp, cp1_x + (mid_width / 2) + chw3, iy, kw, "ZVEL", convertAltString(fs.navTargetRelVel.z));

				// Indicators
				if (fs.docked) {
					skp->SetBrush(brushRed);
					skp->SetPen(penRed);
					skp->SetTextColor(WHITE);
					int tw = skp->GetTextWidth("DOCKED", 6);
					int offset = (int)((cp1_x - x - tw) / 2);
					skp->Rectangle(x + offset, y + height - th, cp1_x - offset, y + height);
					skp->TextBox(x + offset, y + height - th, cp1_x - offset, y + height, "DOCKED", 6);
					offset = (int)((x + width - cp2_x - tw) / 2);
					skp->Rectangle(cp2_x + offset, y + height - th, x + width - offset, y + height);
					skp->TextBox(cp2_x + offset, y + height - th, x + width - offset, y + height, "DOCKED", 6);
				}

				skp->SetBrush(brushGreen2);
				skp->SetPen(penGreen2);
				int y1 = y + (int)(th / 2);
				int y2 = y + height - (int)(1.5*th);
				int h = y2 - y1;
				int x1 = x + (int)((cp1_x - x) / 3);
				int x2 = cp1_x - (int)((cp1_x - x) / 3);
				int w = x2 - x1;
				double ld = min(1000, log10(length(fs.navTargetRelPos)));
				ld += 1;
				if (ld < 0) ld = 0;
				skp->Rectangle(x1, y2 - 1, x2, y2 - (int)(ld * h / 4) - 1);
				skp->SetBrush(NULL);
				skp->SetPen(penWhite);
				skp->Rectangle(x1, y1, x2, y2);
				for (int i = 1; i < 4; i++) {
					skp->Line(x1, y1 + i * (int)(h / 4), x2, y1 + i * (int)(h / 4));
				}

				if (fs.navTargetRelVel.x < 0) {
					skp->SetBrush(brushYellow2);
					skp->SetPen(penYellow2);
				} else {
					skp->SetBrush(brushGreen2);
					skp->SetPen(penGreen2);
				}
				x1 = cp2_x + (int)((x + width - cp2_x) / 3);
				x2 = x + width - (int)((x + width - cp2_x) / 3);
				w = x2 - x1;
				ld = min(1000, log10(length(fs.navTargetRelVel)));
				ld += 1;
				if (ld < 0) ld = 0;
				skp->Rectangle(x1, y2 - 1, x2, y2 - (int)(ld * h / 4) - 1);
				skp->SetBrush(NULL);
				skp->SetPen(penWhite);
				skp->Rectangle(x1, y1, x2, y2);
				for (int i = 1; i < 4; i++) {
					skp->Line(x1, y1 + i * (int)(h / 4), x2, y1 + i * (int)(h / 4));
				}


			}
		}
	}
	else {
		// Row 1
		WriteText(skp, cp1_x + chw3, iy, kw, "ApA", convertAltString(fs.apoapsis));
		WriteText(skp, cp1_x + (mid_width / 2) + chw3, iy, kw, "PeA", convertAltString(fs.periapsis));

		// Row 2
		iy += (chw3 + th);
		WriteText(skp, cp1_x + chw3, iy, kw, "ApT", convertAltString(fs.apoT));
		WriteText(skp, cp1_x + (mid_width / 2) + chw3, iy, kw, "PeT", convertAltString(fs.periT));

		// Row 3
		iy += (chw3 + th);
		s = std::to_string(round(fs.ecc * (double)10000) / (double)10000).substr(0, 6);
		WriteText(skp, cp1_x + chw3, iy, kw, "Ecc", s);
		WriteText(skp, cp1_x + (mid_width / 2) + chw3, iy, kw, "Inc", convertAngleString(fs.inc));

		// Row 4
		iy += (chw3 + th);
		if (frm == 3) {
			double ang = fs.lon;
			s = convertAngleString(abs(ang));
			if (ang < 0) s.append("W"); else s.append("E");
			WriteText(skp, cp1_x + chw3, iy, kw, "Lon", s);
			ang = fs.lat;
			s = convertAngleString(abs(ang));
			if (ang < 0) s.append("S"); else s.append("N");
			WriteText(skp, cp1_x + (mid_width / 2) + chw3, iy, kw, "Lat", s);
		}
		else {
			WriteText(skp, cp1_x + chw3, iy, kw, "LAN", convertAngleString(fs.lan));
			WriteText(skp, cp1_x + (mid_width / 2) + chw3, iy, kw, "T", convertAltString(fs.t));
		}
	}

	oapiReleaseFont(f);
	oapiReleaseFont(fsmall);
	oapiReleaseFont(fxsmall);
	//oapiReleaseFont(fxxsmall);
}

std::string AttitudeIndicatorMFD::convertAngleString(double angle) {
	double ang = angle*DEG;
	double scale = 10;
	if (ang < 100) scale = 100;
	if (ang < 10) scale = 1000;
	std::string s = std::to_string(round(ang * scale) / scale);
	if (ang < 0)
		s = s.substr(0, 6);
	else
		s = s.substr(0, 5);
	s.append("°");
	return s;
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
	if (abs(altitude) > 1000) {
		s_suff = "G";
		altitude /= 1000; // in G
	}
	if (abs(altitude) > 1000) {
		s_suff = "T";
		altitude /= 1000; // in T
	}
	double scale = 1;
	if (abs(altitude) < 1000) scale = 10;
	if (abs(altitude) < 100) scale = 100;
	if (abs(altitude) < 10) scale = 1000;
	std::string s;
	s.append(std::to_string(round(altitude * scale) / scale));
	if (altitude < 0)
		s = s.substr(0, 6);
	else
		s = s.substr(0, 5);
	s.append(s_suff);
	return s;
}

// MFD message parser
int AttitudeIndicatorMFD::MsgProc(UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam)
{
	//std::string s = "[AttitudeIndicatorMFD] Enter: MsgProc: ";
	//s.append(std::to_string(msg));
	//char  buf[50];
	//strcpy(buf, s.c_str());
	//oapiWriteLog(buf);
	switch (msg) {
	case OAPI_MSG_MFD_OPENED:
		// Our new MFD mode has been selected, so we create the MFD and
		// return a pointer to it.
		return (int)new AttitudeIndicatorMFD(LOWORD(wparam), HIWORD(wparam), (VESSEL*)lparam);;
	}
	return 0;
}