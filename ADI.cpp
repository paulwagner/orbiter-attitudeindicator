#include "ADI.h"
#include "AttitudeReferenceADI.h"
#include <sstream>
#include "Configuration.h"
#include "imageloader.h"
#include "commons.h"

ADI::ADI(int x, int y, int width, int height, AttitudeReferenceADI* attref, double cw, double ch, CONFIGURATION& config, MFDSettings* settings) {
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
	this->attref = attref;
	this->settings = settings;
	this->cw = cw;
	this->ch = ch;
	this->cw_i = (int)round(cw);
	this->cw3_i = (int)round(cw / 3);
	this->cw2_i = (int)round(cw / 2);
	this->cw23 = cw * 2 / 3;
	this->cw23_i = (int)round(cw23);
	this->ch_i = (int)round(ch);
	this->ch3_i = (int)round(ch / 3);
	this->ch2_i = (int)round(ch / 2);
	this->ch23 = ch * 2 / 3;
	this->ch23_i = (int)round(ch23);

	int p1 = max((int)round(cw / 6),1);
	int p2 = max((int)round(cw / 8), 1);
	penWing = oapiCreatePen(1, p1, config.wingColor);
	penTurnVec = oapiCreatePen(1, p2, config.turnVecColor);
	penGrad = oapiCreatePen(1, p1, config.progradeColor);
	penNormal = oapiCreatePen(1, p1, config.normalColor);
	penRadial = oapiCreatePen(1, p1, config.radialColor);
	penPerpendicular = oapiCreatePen(1, p1, config.perpendicularColor);
	penTarget = oapiCreatePen(1, p1, config.targetColor);
	penManeuver = oapiCreatePen(1, p1, config.maneuverColor);
	penIndicators = oapiCreatePen(1, p2, config.indicatorColor);
	brushWing = oapiCreateBrush(config.wingColor);
	brushTurnVec = oapiCreateBrush(config.turnVecColor);
	brushGrad = oapiCreateBrush(config.progradeColor);
	brushNormal = oapiCreateBrush(config.normalColor);
	brushRadial = oapiCreateBrush(config.radialColor);
	brushPerpendicular = oapiCreateBrush(config.perpendicularColor);
	brushTarget = oapiCreateBrush(config.targetColor);
	brushManeuver = oapiCreateBrush(config.maneuverColor);
	brushIndicators = oapiCreateBrush(config.indicatorColor);

	GLuint PixelFormat;
	BITMAPINFOHEADER BIH;
	BIH.biSize = sizeof(BITMAPINFOHEADER);
	BIH.biWidth=width;
	BIH.biHeight=height;
	BIH.biPlanes=1;
	BIH.biBitCount=16;//default is 16.
	BIH.biCompression=BI_RGB;
	BIH.biSizeImage=0;
	void* m_pBits;
	hDC=CreateCompatibleDC(NULL);//we make a new DC and DIbitmap for OpenGL to draw onto
	static  PIXELFORMATDESCRIPTOR pfd2;
	DescribePixelFormat(hDC,1,sizeof(PIXELFORMATDESCRIPTOR),&pfd2);//just get a random pixel format.. 
	BIH.biBitCount=pfd2.cColorBits;//to get the current bit depth.. !?
	hBMP=CreateDIBSection(hDC,(BITMAPINFO*)&BIH,DIB_RGB_COLORS,&m_pBits,NULL,0);
	hBMP_old=(HBITMAP)SelectObject(hDC,hBMP);
	static  PIXELFORMATDESCRIPTOR pfd={                             // pfd Tells Windows How We Want Things To Be
        sizeof(PIXELFORMATDESCRIPTOR),                              // Size Of This Pixel Format Descriptor
        1,                                                          // Version Number
		PFD_DRAW_TO_BITMAP |                                        // Format Must Support Bitmap Rendering
        PFD_SUPPORT_OPENGL |									
		PFD_SUPPORT_GDI,											// Format Must Support OpenGL,                                           
		0,//        PFD_TYPE_RGBA,                                              // Request An RGBA Format
        16,															// Select Our Color Depth
        0, 0, 0, 0, 0, 0,                                           // Color Bits Ignored
        0,//1,                                                          // No Alpha Buffer
        0,                                                          // Shift Bit Ignored
        0,                                                          // No Accumulation Buffer
        0, 0, 0, 0,                                                 // Accumulation Bits Ignored
        0,//16,                                                         // 16Bit Z-Buffer (Depth Buffer)  
        0,                                                          // No Stencil Buffer
        0,                                                          // No Auxiliary Buffer
        0,//PFD_MAIN_PLANE,                                             // Main Drawing Layer
        0,                                                          // Reserved
        0, 0, 0                                                     // Layer Masks Ignored
    };
	pfd.cColorBits=pfd2.cColorBits;//same color depth needed.
	PixelFormat=ChoosePixelFormat(hDC,&pfd);// now pretend we want a new format
	SetPixelFormat(hDC,PixelFormat,&pfd);
	hRC=wglCreateContext(hDC);
	wglMakeCurrent(hDC,hRC);					//all standard OpenGL init so far

	textureId = 0;
	Image* texture = loadBMP(config.texturePath);
	
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, width, height);
	//glClearColor(0.0f, 0.0f, 0.0f, 0.0f);           // Panel Background color

	if (texture) {
		glEnable(GL_NORMALIZE);
		glEnable(GL_COLOR_MATERIAL);
		quad = gluNewQuadric();
		glGenTextures(1, &textureId); //Make room for our texture
		glBindTexture(GL_TEXTURE_2D, textureId); //Tell OpenGL which texture to edit
		//Map the image to the texture
		glTexImage2D(GL_TEXTURE_2D,                //Always GL_TEXTURE_2D
			0,                            //0 for now
			GL_RGB,                       //Format OpenGL uses for image
			texture->width, texture->height,  //Width and height
			0,                            //The border of the image
			GL_RGB, //GL_RGB, because pixels are stored in RGB format
			GL_UNSIGNED_BYTE, //GL_UNSIGNED_BYTE, because pixels are stored as unsigned numbers
			texture->pixels);               //The actual pixel data
		delete texture;
	}
}

ADI::~ADI() {
	oapiReleasePen(penWing);
	oapiReleasePen(penTurnVec);
	oapiReleasePen(penGrad);
	oapiReleasePen(penNormal);
	oapiReleasePen(penRadial);
	oapiReleasePen(penPerpendicular);
	oapiReleasePen(penTarget);
	oapiReleasePen(penManeuver);
	oapiReleasePen(penIndicators);
	oapiReleaseBrush(brushWing);
	oapiReleaseBrush(brushTurnVec);
	oapiReleaseBrush(brushGrad);
	oapiReleaseBrush(brushNormal);
	oapiReleaseBrush(brushRadial);
	oapiReleaseBrush(brushPerpendicular);
	oapiReleaseBrush(brushTarget);
	oapiReleaseBrush(brushManeuver);
	oapiReleaseBrush(brushIndicators);
	if (quad)
		gluDeleteQuadric(quad);
	wglMakeCurrent(NULL, NULL);	//standard OpenGL release
	wglDeleteContext(hRC);
	hRC=NULL;
	SelectObject(hDC,hBMP_old);//remember to delete DC and bitmap memory we created
	DeleteObject(hBMP);
	DeleteDC(hDC);
}

void ADI::DrawBall(oapi::Sketchpad* skp, double zoom) {
	if (textureId == 0){
		skp->Text(5, height / 2, "Texture not found!", 18);
		return;
	}

	wglMakeCurrent(hDC, hRC);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);             // Clear The Screen And The Depth Buffer        

	diameter = zoom * min(width, height);
	double zoomd = 1.0 / zoom;
	double aspect = (double)width / (double)height;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if (aspect > 1)  // wider than tall
		glOrtho(-zoomd * aspect, zoomd * aspect, -zoomd, zoomd, 2.0, 4.0);
	else
		glOrtho(-zoomd, zoomd, -zoomd / aspect, zoomd / aspect, 2.0, 4.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -3.0f);
	double m[16];
	GetOpenGLRotMatrix(m);
	glMultMatrixd(m);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureId);

	glPushMatrix();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glRotatef(-90, 1.0f, 0.0f, 0.0f); // Get texture to right position
	gluQuadricTexture(quad, 1);
	gluQuadricOrientation(quad, GLU_OUTSIDE);
	gluSphere(quad, 1, 40, 40);
	glPopMatrix();

	glFlush();
	glFinish();
	HDC	skphDC = skp->GetDC();
	if (skphDC != NULL) {
		BitBlt(skphDC, x, y, width, height, this->hDC, 0, 0, SRCCOPY);
	}
	else {
		// Workaround for newer D3D9 clients
		SURFHANDLE surf = skp->GetSurface();
		SURFHANDLE newSurf = oapiCreateSurfaceEx(width, height, OAPISURFACE_SYSMEM);
		HDC newSurfDC = oapiGetDC(newSurf);
		BitBlt(newSurfDC, x, y, width, height, this->hDC, 0, 0, SRCCOPY);
		oapiReleaseDC(newSurf, newSurfDC);

		oapiBlt(surf, newSurf, x, y, 0, 0, width, height);
		oapiDestroySurface(newSurf);
	}

	DrawVectors(skp);
	if (settings->turnVectorMode == 1)
		DrawRateIndicators(skp);
	if (settings->turnVectorMode == 2)
		DrawTurnVector(skp);
	DrawWing(skp);
}

void ADI::GetOpenGLRotMatrix(double* m) {
	VECTOR3 eulerangles = attref->GetEulerAngles();
	double rho = eulerangles.x; // roll angle
	double tht = eulerangles.y; // pitch angle
	double phi = eulerangles.z; // yaw angle

	double sinp = sin(phi), cosp = cos(phi);
	double sint = sin(tht), cost = cos(tht);
	double sinr = sin(rho), cosr = cos(rho);

	// Ball transformation
	if (attref->GetProjMode() == 0) {
		// below are the coefficients of the rows of the rotation matrix M for the ball,
		// given by M = RTP, with
		// MATRIX3 P = {cosp,0,-sinp,  0,1,0,  sinp,0,cosp};
		// MATRIX3 T = {1,0,0,  0,cost,-sint,  0,sint,cost};
		// MATRIX3 R = {cosr,sinr,0,  -sinr,cosr,0,  0,0,1};

		m[0] = cosr*cosp - sinr*sint*sinp;
		m[4] = sinr*cost;
		m[8] = -cosr*sinp - sinr*sint*cosp;
		m[1] = -sinr*cosp - cosr*sint*sinp;
		m[5] = cosr*cost;
		m[9] = sinr*sinp - cosr*sint*cosp;
		m[2] = cost*sinp;
		m[6] = sint;
		m[10] = cost*cosp;

	}
	else {
		// below are the coefficients of the rows of the rotation matrix M for the ball,
		// given by M = ZRPT, with
		// MATRIX3 Z = {0,1,0,  -1,0,0,  0,0,1};
		// MATRIX3 P = {1,0,0,  0,cosp,-sinp,  0,sinp,cosp};  // yaw
		// MATRIX3 T = {cost,0,sint,  0,1,0,  -sint,0,cost};  // pitch
		// MATRIX3 R = {cosr,sinr,0,  -sinr,cosr,0,  0,0,1};  // bank

		m[0] = -sinr*cost + cosr*sinp*sint;
		m[4] = cosr*cosp;
		m[8] = -sinr*sint - cosr*sinp*cost;
		m[1] = -cosr*cost - sinr*sinp*sint;
		m[5] = -sinr*cosp;
		m[9] = -cosr*sint + sinr*sinp*cost;
		m[2] = -cosp*sint;
		m[6] = sinp;
		m[10] = cosp*cost;
	}

	m[3] = 0;
	m[7] = 0;
	m[11] = 0;
	m[12] = 0;
	m[13] = 0;
	m[14] = 0;
	m[15] = 1;
}

void ADI::DrawWing(oapi::Sketchpad* skp) {
	skp->SetPen(penWing);
	int w2 = x + (int)round(width / 2);
	int h2 = y + (int)round(height / 2);
	if (attref->IsDockRef() && attref->GetMode() == 4 && attref->GetFlightStatus().navType == TRANSMITTER_IDS) {
		int tx = (int)round(cw * 2 / 5);
		int ty = (int)round(cw * 2 / 5);
		skp->SetBrush(NULL);
		skp->Ellipse(w2 - tx, h2 - ty, w2 + tx, h2 + ty);
		skp->Ellipse(w2 - 2 * tx, h2 - 2 * ty, w2 + 2 * tx, h2 + 2 * ty);
		skp->Line(w2 + tx, h2, w2 + 3 * tx, h2);
		skp->Line(w2 - tx, h2, w2 - 3 * tx, h2);
		skp->Line(w2, h2 + ty, w2, h2 + 3 * ty);
		skp->Line(w2, h2 - ty, w2, h2 - 3 * ty);
		return;
	}
	skp->SetBrush(brushWing);
	skp->MoveTo(w2 - 3 * cw_i, h2);
	skp->LineTo(w2 - cw_i, h2);
	skp->LineTo(w2, h2 + ch23_i);
	skp->LineTo(w2 + cw_i, h2);
	skp->LineTo(w2 + 3 * cw_i, h2);
	skp->Rectangle(w2 - 1, h2 - 1, w2 + 1, h2 + 1);
}

void ADI::DrawTurnVector(oapi::Sketchpad* skp) {
	FLIGHTSTATUS fs = attref->GetFlightStatus();
	double yawProj = (1 + fs.yawrate*RAD) * ((double)width / 2);
	double pitchProj = (1 + fs.pitchrate*RAD) * ((double)height / 2);
	CheckRange(yawProj, (double)0, (double)width);
	CheckRange(pitchProj, (double)0, (double)height);
	pitchProj = height - pitchProj; // invert y coords
	double roll = abs(fs.rollrate*RAD); // Between 0 and 1

	// Draw vector
	skp->SetPen(penTurnVec);
	skp->SetBrush(0);
	skp->MoveTo(width / 2, height / 2);
	skp->LineTo(int(yawProj+0.5), int(pitchProj+0.5));
	int offset = int((roll*min(width, height) / 2) + 0.5);
	skp->Ellipse(width / 2 - offset, height / 2 - offset, width / 2 + offset, height / 2 + offset);
}

void ADI::ProjectVector(VECTOR3 vector, double& x, double& y, double &phi) {
	GLdouble model[16];
	GLdouble proj[16];
	GLint view[4];
	GLdouble z;
	GLdouble fp[3];
	glGetDoublev(GL_MODELVIEW_MATRIX, model);
	glGetDoublev(GL_PROJECTION_MATRIX, proj);
	glGetIntegerv(GL_VIEWPORT, view);

	// View coordinates
	VECTOR3 cov;
	normalise(vector);
	gluUnProject((GLdouble)width / 2, (GLdouble)height / 2, 0, model, proj, view, &cov.x, &cov.y, &cov.z);
	phi = acos(dotp(vector, cov) / length(cov))*DEG;
	fp[0] = vector.x;
	fp[1] = vector.y;
	fp[2] = vector.z;

	//(0,0,1) = N; (0,1,0) = H; (1,0,0) = E
	gluProject(fp[0], fp[1], fp[2], model, proj, view, &x, &y, &z);

	CheckRange(x, (double)0, (double)width);
	CheckRange(y, (double)0, (double)height);
	y = height - y; // invert y coords
}

void ADI::DrawDirectionArrow(oapi::Sketchpad* skp, oapi::IVECTOR2 v) {
	double s1 = cw / 3; // Radial size of arrow
	double s2 = (cw / 5) * RAD; // Radial width of arrow
	v.x -= (int)round(width / 2);
	v.y -= (int)round(height / 2);
	// To polar coordinates
	double phi = atan2(v.y, v.x);
	double r = diameter / 2.05;
	// Back to sketchpad coordinates
	oapi::IVECTOR2 pts[4];
	pts[0].x = (int)round(r * cos(phi)); pts[0].y = (int)round(r * sin(phi));
	pts[1].x = (int)round((r - 3 * s1) * cos(phi - s2)); pts[1].y = (int)round((r - 3 * s1) * sin(phi - s2));
	pts[2].x = (int)round((r - 2 * s1) * cos(phi)); pts[2].y = (int)round((r - 2 * s1) * sin(phi));
	pts[3].x = (int)round((r - 3 * s1) * cos(phi + s2)); pts[3].y = (int)round((r - 3 * s1)* sin(phi + s2));
	for (int i = 0; i < 4; i++) {
		pts[i].x += (int)round(width / 2);
		pts[i].y += (int)round(height / 2);
	}
	// Check range
	int x_offs = CheckRange(pts[0].x, (long)0, (long)width);
	int y_offs = CheckRange(pts[0].y, (long)0, (long)height);
	for (int i = 1; i < 4; i++) {
		pts[i].x -= x_offs;
		pts[i].y -= y_offs;
	}
	skp->Polygon(pts, 4);
}

void DrawArc(oapi::Sketchpad* skp, double x, double y, double r, double s, double e) {
	double xcoords[360];
	double ycoords[360];

	for (int i = 0; i<360; i++){
		double degree = i*RAD;
		xcoords[i] = x + (r*sin(degree));
		ycoords[i] = y - (r*cos(degree));
	}
	for (int i = 1; i<360; i++) {
		if (i >= s && i <= e) {
			int px = (int)xcoords[i];
			int py = (int)ycoords[i];
			skp->Ellipse(px - 1, py - 1, px + 1, py + 1);
		}
	}
}

void ADI::DrawVectors(oapi::Sketchpad* skp) {
	const VESSEL *v = attref->GetVessel();
	FLIGHTSTATUS fs = attref->GetFlightStatus();
	VECTOR3 pgd, nml, rad, pep, tgt, man;
	int frm = attref->GetMode();
	double phi;
	double x, y;
	int ix, iy;
	double phiF = 70; // max marker view angle

	// Calculate direction vectors
	if (frm == 2)
		attref->GetOrbitalSpeedDirection(pgd, nml, rad, pep); // Orbital relative vector in OV/OM
	else if (frm == 3)
		attref->GetAirspeedDirection(pgd, nml, rad, pep); // Surface relative vector in LH/LN
	else if (frm == 4 && fs.hasNavTarget)
		attref->GetTargetDirections(tgt, pgd); // Target relative vector in NAV
	if (frm <= 2)
		attref->GetManeuverDirections(man);

	int cx = cw23_i;
	double cxd = cw23;
	int cy = ch23_i;
	double cyd = ch23;

	if (frm == 4){
		// Target marker
		if (fs.hasNavTarget && isnormal(length(tgt)) && (!fs.docked || !attref->IsDockRef())){
			int tx = (int)round(cxd / 2);
			int ty = (int)round(cyd / 2);
			ProjectVector(tgt, x, y, phi);
			ix = (int)x, iy = (int)y;
			bool tgtVisible = false;
			oapi::IVECTOR2 tgtDir; tgtDir.x = ix; tgtDir.y = iy;
			if (abs(phi) <= phiF) {
				tgtVisible = true;
				skp->SetPen(penTarget);
				skp->SetBrush(brushTarget);
				skp->Ellipse(ix - tx / 2, iy - ty / 2, ix + tx / 2, iy + ty / 2);
				skp->SetBrush(NULL);
				DrawArc(skp, ix + tx, iy - ty, cx, 0, 90);
				DrawArc(skp, ix + tx, iy + ty, cx, 90, 180);
				DrawArc(skp, ix - tx, iy + ty, cx, 180, 270);
				DrawArc(skp, ix - tx, iy - ty, cx, 270, 360);
			}
			// Anti-target
			ProjectVector(-tgt, x, y, phi);
			ix = (int)x, iy = (int)y;
			if (abs(phi) <= phiF) {
				double dx = sin(60 * RAD);
				double dy = cos(60 * RAD);
				skp->SetPen(penTarget);
				skp->SetBrush(brushTarget);
				skp->Ellipse(ix - tx / 2, iy - ty / 2, ix + tx / 2, iy + ty / 2);
				skp->SetBrush(NULL);
				skp->Line(ix, iy + cy, ix, iy + 2 * cy);
				skp->Line(ix + (int)round(cxd * dx), iy - (int)round(cyd*dy), ix + 2 * (int)round(cxd * dx), iy - 2 * (int)round(cyd*dy));
				skp->Line(ix - (int)round(cxd * dx), iy - (int)round(cyd*dy), ix - 2 * (int)round(cxd * dx), iy - 2 * (int)round(cyd*dy));
			}
			else if (!tgtVisible){
				// No marker visible, draw direction arrow to target
				skp->SetPen(penTarget);
				skp->SetBrush(brushTarget);
				DrawDirectionArrow(skp, tgtDir);
			}
		}
	
		// Draw course marker
		if (fs.navType == TRANSMITTER_ILS || (fs.navType == TRANSMITTER_NONE && attref->GetVessel()->GetAtmRef() != 0) || fs.navType == TRANSMITTER_VOR) {
			int tx = cx;
			int ty = cy;
			double crs = fs.navCrs[attref->GetNavid()];
			VECTOR3 chvec;
			double sinp = sin(crs), cosp = cos(crs);
			if (attref->GetProjMode() == 0)
				chvec = _V(RAD*sinp, 0, RAD*cosp);
			else
				chvec = _V(0, RAD*sinp, RAD*cosp);
			ProjectVector(chvec, x, y, phi);
			ix = (int)x, iy = (int)y;
			if (abs(phi) <= phiF) {
				double dx = sin(60 * RAD);
				double dy = sin(30 * RAD);
				double crsScale = 1.5;
				skp->SetPen(penPerpendicular);
				skp->SetBrush(NULL);
				skp->Line(ix, iy - (int)round(cyd * 2 / 3), ix, iy + (int)round(cyd / 3));
				skp->Line(ix - (int)round(cxd*dx*crsScale), iy + (int)round(cyd*dy*crsScale), ix + (int)round(cxd*dx*crsScale), iy + (int)round(cyd*dy*crsScale));
				skp->Line(ix - (int)round(cxd*dx*crsScale), iy + (int)round(cyd*dy*crsScale), ix, iy - (int)round(cyd*crsScale));
				skp->Line(ix, iy - (int)round(cyd*crsScale), ix + (int)round(cxd*dx*crsScale), iy + (int)round(cyd*dy*crsScale));
			}
		}

		return; // No normal/radial markers in NAV mode	
	}

	// Prograde
	if (frm > 1 && settings->drawPrograde && (frm != 4 || (fs.hasNavTarget && (fs.navType == TRANSMITTER_IDS || fs.navType == TRANSMITTER_XPDR || fs.navType == TRANSMITTER_VTOL))) && isnormal(length(pgd))) {
		ProjectVector(pgd, x, y, phi);
		ix = (int)x, iy = (int)y;
		bool pgdVisible = false;
		oapi::IVECTOR2 pgdDir; pgdDir.x = ix; pgdDir.y = iy;
		if (abs(phi) <= phiF) {
			pgdVisible = true;
			skp->SetPen(penGrad);
			skp->SetBrush(brushGrad);
			skp->Ellipse(ix - 1, iy - 1, ix + 1, iy + 1);
			skp->SetBrush(NULL);
			skp->Ellipse(ix - cx, iy - cy, ix + cx, iy + cy);
			skp->Line(ix, iy - cy, ix, iy - 2 * cy);
			skp->Line(ix + cx, iy, ix + 2 * cx, iy);
			skp->Line(ix - cx, iy, ix - 2 * cx, iy);
		}
		// Retrograde
		ProjectVector(-pgd, x, y, phi);
		ix = (int)x, iy = (int)y;
		if (abs(phi) <= phiF) {
			double d = sin(45 * RAD);
			skp->SetPen(penGrad);
			skp->SetBrush(NULL);
			skp->Ellipse(ix - cx, iy - cy, ix + cx, iy + cy);
			skp->Line(ix, iy - cy, ix, iy - 2 * cy);
			skp->Line(ix + cx, iy, ix + 2 * cx, iy);
			skp->Line(ix - cx, iy, ix - 2 * cx, iy);
			skp->Line(ix - (int)round(cxd * d), iy - (int)round(cyd * d), ix + (int)round(cxd * d), iy + (int)round(cyd * d));
			skp->Line(ix - (int)round(cxd * d), iy + (int)round(cyd * d), ix + (int)round(cxd * d), iy - (int)round(cyd * d));
		}
		else if (!pgdVisible){
			// No marker visible, draw direction arrow to prograde
			skp->SetPen(penGrad);
			skp->SetBrush(brushGrad);
			DrawDirectionArrow(skp, pgdDir);
		}
	}

	// Normal
	if (frm > 1 && settings->drawNormal && isnormal(length(pgd))){
		double dx = sin(60 * RAD);
		double dy = sin(30 * RAD);
		double nmlScale = 1.5;
		ProjectVector(nml, x, y, phi);
		ix = (int)x, iy = (int)y;
		bool nmlVisible = false;
		oapi::IVECTOR2 nmlDir; nmlDir.x = ix; nmlDir.y = iy;
		if (abs(phi) <= phiF) {
			nmlVisible = true;
			skp->SetBrush(brushNormal);
			skp->SetPen(penNormal);
			skp->Ellipse(ix - 1, iy - 1, ix + 1, iy + 1);
			skp->Line(ix - (int)round(cxd*dx*nmlScale), iy + (int)round(cyd*dy*nmlScale), ix + (int)round(cxd*dx*nmlScale), iy + (int)round(cyd*dy*nmlScale));
			skp->Line(ix - (int)round(cxd*dx*nmlScale), iy + (int)round(cyd*dy*nmlScale), ix, iy - (int)round(cyd*nmlScale));
			skp->Line(ix, iy - (int)round(cyd*nmlScale), ix + (int)round(cxd*dx*nmlScale), iy + (int)round(cyd*dy*nmlScale));
		}
		// Anti-Normal
		ProjectVector(-nml, x, y, phi);
		ix = (int)x, iy = (int)y;
		if (abs(phi) <= phiF) {
			skp->SetBrush(brushNormal);
			skp->SetPen(penNormal);
			skp->Ellipse(ix - 1, iy - 1, ix + 1, iy + 1);
			oapi::IVECTOR2 a1; a1.x = ix - (int)round(cxd*dx*nmlScale); a1.y = iy - (int)round(cyd*dy*nmlScale);
			oapi::IVECTOR2 a2; a2.x = ix + (int)round(cxd*dx*nmlScale); a2.y = iy - (int)round(cyd*dy*nmlScale);
			oapi::IVECTOR2 a3; a3.x = ix; a3.y = iy + (int)round(cyd*nmlScale);
			skp->Line(a1.x, a1.y, a2.x, a2.y);
			skp->Line(a1.x, a1.y, a3.x, a3.y);
			skp->Line(a3.x, a3.y, a2.x, a2.y);
			oapi::IVECTOR2 p1; p1.x = (a1.x + a3.x)/2; p1.y = (a1.y + a3.y)/2;
			oapi::IVECTOR2 p2; p2.x = (a2.x + a3.x) / 2; p2.y = (a2.y + a3.y) / 2;
			oapi::IVECTOR2 p3; p3.x = (a1.x + a2.x) / 2; p3.y = (a1.y + a2.y) / 2;
			oapi::IVECTOR2 q1; q1.x = 2 * p1.x - ix; q1.y = 2 * p1.y - iy;
			oapi::IVECTOR2 q2; q2.x = 2 * p2.x - ix; q2.y = 2 * p2.y - iy;
			oapi::IVECTOR2 q3; q3.x = 2 * p3.x - ix; q3.y = 2 * p3.y - iy;
			skp->Line(p1.x, p1.y, q1.x, q1.y);
			skp->Line(p2.x, p2.y, q2.x, q2.y);
			skp->Line(p3.x, p3.y, q3.x, q3.y);
		} else if (!nmlVisible){
			// No marker visible, draw direction arrow to normal
			skp->SetPen(penNormal);
			skp->SetBrush(brushNormal);
			DrawDirectionArrow(skp, nmlDir);
		}
	}

	// Radial out
	if (frm > 1 && settings->drawRadial && isnormal(length(pgd))) {
		double sd = sin(45 * RAD);
		double cd = cos(45 * RAD);
		double radScale = 1.6;
		ProjectVector(rad, x, y, phi);
		ix = (int)x, iy = (int)y;
		bool radVisible = false;
		oapi::IVECTOR2 radDir; radDir.x = ix; radDir.y = iy;
		if (abs(phi) <= phiF) {
			radVisible = true;
			skp->SetPen(penRadial);
			skp->SetBrush(brushRadial);
			skp->Ellipse(ix - 1, iy - 1, ix + 1, iy + 1);
			skp->SetBrush(NULL);
			skp->Ellipse(ix - cx, iy - cy, ix + cx, iy + cy);
			skp->Line(ix + (int)round(cxd*cd), iy + (int)round(cyd*sd), ix + (int)round(radScale * cxd*cd), iy + (int)round(radScale * cyd*sd));
			skp->Line(ix + (int)round(cxd*cd), iy - (int)round(cyd*sd), ix + (int)round(radScale * cxd*cd), iy - (int)round(radScale * cyd*sd));
			skp->Line(ix - (int)round(cxd*cd), iy + (int)round(cyd*sd), ix - (int)round(radScale * cxd*cd), iy + (int)round(radScale * cyd*sd));
			skp->Line(ix - (int)round(cxd*cd), iy - (int)round(cyd*sd), ix - (int)round(radScale * cxd*cd), iy - (int)round(radScale * cyd*sd));
		}
		// Radial in
		ProjectVector(-rad, x, y, phi);
		ix = (int)x, iy = (int)y;
		if (abs(phi) <= phiF) {
			skp->SetPen(penRadial);
			skp->SetBrush(brushRadial);
			skp->Ellipse(ix - 1, iy - 1, ix + 1, iy + 1);
			skp->SetBrush(NULL);
			skp->Ellipse(ix - cx, iy - cy, ix + cx, iy + cy);
			skp->Line(ix + (int)round(cxd*cd), iy + (int)round(cyd*sd), ix + (int)round((2 - radScale) * cxd*cd), iy + (int)round((2 - radScale) * cyd*sd));
			skp->Line(ix + (int)round(cxd*cd), iy - (int)round(cyd*sd), ix + (int)round((2 - radScale) * cxd*cd), iy - (int)round((2 - radScale) * cyd*sd));
			skp->Line(ix - (int)round(cxd*cd), iy + (int)round(cyd*sd), ix - (int)round((2 - radScale) * cxd*cd), iy + (int)round((2 - radScale) * cyd*sd));
			skp->Line(ix - (int)round(cxd*cd), iy - (int)round(cyd*sd), ix - (int)round((2 - radScale) * cxd*cd), iy - (int)round((2 - radScale) * cyd*sd));
		}
		else if (!radVisible){
			// No marker visible, draw direction arrow to radial
			skp->SetPen(penRadial);
			skp->SetBrush(brushRadial);
			DrawDirectionArrow(skp, radDir);
		}
	}

	// Perpendicular out
	if (frm > 1 && settings->drawPerpendicular && isnormal(length(pgd))) {
		double sd = sin(45 * RAD);
		double cd = cos(45 * RAD);
		double pepScale = 1.6;
		ProjectVector(pep, x, y, phi);
		ix = (int)x, iy = (int)y;
		bool pepVisible = false;
		oapi::IVECTOR2 pepDir; pepDir.x = ix; pepDir.y = iy;
		if (abs(phi) <= phiF) {
			pepVisible = true;
			skp->SetPen(penPerpendicular);
			skp->SetBrush(brushPerpendicular);
			skp->Ellipse(ix - 1, iy - 1, ix + 1, iy + 1);
			skp->SetBrush(NULL);
			skp->Ellipse(ix - cx, iy - cy, ix + cx, iy + cy);
			skp->Line(ix + (int)round(cxd*cd), iy + (int)round(cyd*sd), ix + (int)round(pepScale * cxd*cd), iy + (int)round(pepScale * cyd*sd));
			skp->Line(ix + (int)round(cxd*cd), iy - (int)round(cyd*sd), ix + (int)round(pepScale * cxd*cd), iy - (int)round(pepScale * cyd*sd));
			skp->Line(ix - (int)round(cxd*cd), iy + (int)round(cyd*sd), ix - (int)round(pepScale * cxd*cd), iy + (int)round(pepScale * cyd*sd));
			skp->Line(ix - (int)round(cxd*cd), iy - (int)round(cyd*sd), ix - (int)round(pepScale * cxd*cd), iy - (int)round(pepScale * cyd*sd));
		}
		// Perpendicular in
		ProjectVector(-pep, x, y, phi);
		ix = (int)x, iy = (int)y;
		if (abs(phi) <= phiF) {
			skp->SetPen(penPerpendicular);
			skp->SetBrush(brushPerpendicular);
			skp->Ellipse(ix - 1, iy - 1, ix + 1, iy + 1);
			skp->SetBrush(NULL);
			skp->Ellipse(ix - cx, iy - cy, ix + cx, iy + cy);
			skp->Line(ix + (int)round(cxd*cd), iy + (int)round(cyd*sd), ix + (int)round((2 - pepScale) * cxd*cd), iy + (int)round((2 - pepScale) * cyd*sd));
			skp->Line(ix + (int)round(cxd*cd), iy - (int)round(cyd*sd), ix + (int)round((2 - pepScale) * cxd*cd), iy - (int)round((2 - pepScale) * cyd*sd));
			skp->Line(ix - (int)round(cxd*cd), iy + (int)round(cyd*sd), ix - (int)round((2 - pepScale) * cxd*cd), iy + (int)round((2 - pepScale) * cyd*sd));
			skp->Line(ix - (int)round(cxd*cd), iy - (int)round(cyd*sd), ix - (int)round((2 - pepScale) * cxd*cd), iy - (int)round((2 - pepScale) * cyd*sd));
		}
		else if (!pepVisible){
			// No marker visible, draw direction arrow to perpendicular
			skp->SetPen(penPerpendicular);
			skp->SetBrush(brushPerpendicular);
			DrawDirectionArrow(skp, pepDir);
		}
	}

	// Maneuver marker
	if (fs.hasManRot && frm <= 2) {
		sprintf(oapiDebugString(), "x: %f, y: %f, z: %f", man.x, man.y, man.z);
		ProjectVector(man, x, y, phi);
		ix = (int)x, iy = (int)y;
		oapi::IVECTOR2 manDir; manDir.x = ix; manDir.y = iy;
		skp->SetPen(penManeuver);
		skp->SetBrush(brushManeuver);
		if (abs(phi) <= phiF) {
			double dx = sin(60 * RAD);
			double dy = sin(30 * RAD);
			double _cxd = cxd * 2 / 3;
			double _cyd = cyd * 2 / 3;
			int _cx = (int)round(_cxd);
			int _cy = (int)round(_cyd);
			int rcx = (int)round(_cxd * dx);
			int rcy = (int)round(_cyd * dy);
			skp->Ellipse(ix - 1, iy - 1, ix + 1, iy + 1);
			skp->SetBrush(NULL);
			skp->Line(ix, iy - _cy, ix, iy - 2 * _cy);
			skp->Line(ix + rcx, iy + rcy, ix + 2 * rcx, iy + 2 * rcy);
			skp->Line(ix - rcx, iy + rcy, ix - 2 * rcx, iy + 2 * rcy);
			skp->Line(ix, iy - 2 * _cy, ix + (int)round(_cx / 1.5), iy - 2 * _cy);
			skp->Line(ix, iy - 2 * _cy, ix - (int)round(_cx / 1.5), iy - 2 * _cy);

			int dcx = (int)round(_cxd * sin(45 * RAD) / 1.5);
			int dcy = (int)round(_cyd * cos(45 * RAD) / 1.5);
			skp->Line(ix + 2 * rcx, iy + 2 * rcy, ix + 2 * rcx + dcx, iy + 2 * rcy - dcy);
			skp->Line(ix + 2 * rcx, iy + 2 * rcy, ix + 2 * rcx - dcx, iy + 2 * rcy + dcy);
			skp->Line(ix - 2 * rcx, iy + 2 * rcy, ix - 2 * rcx - dcx, iy + 2 * rcy - dcy);
			skp->Line(ix - 2 * rcx, iy + 2 * rcy, ix - 2 * rcx + dcx, iy + 2 * rcy + dcy);
		}
		else {
			// No marker visible, draw direction arrow to maneuver node
			DrawDirectionArrow(skp, manDir);
		}
	}

}

void ADI::DrawRateIndicators(oapi::Sketchpad* skp) {
	skp->SetPen(penIndicators);
	skp->SetBrush(NULL);
	int border = (int)round(ch*2/3);
	int rwidth = (int)round(ch*1.5);
	skp->Rectangle(width - border - rwidth, height / 4, width - border, height * 3 / 4);
	skp->Line(width - border - rwidth - 1, height / 2, width - border - 1, height / 2);
	skp->Rectangle(width / 4, border, width * 3 / 4, border + rwidth);
	skp->Line(width / 2, border - 1, width / 2, border + rwidth - 1);
	skp->Rectangle(width / 4, height - border - rwidth, width * 3 / 4, height - border);
	skp->Line(width / 2, height - border - 1, width / 2, height - border - rwidth - 1);

	skp->SetBrush(brushIndicators);
	FLIGHTSTATUS fs = attref->GetFlightStatus();
	double spitch = fs.pitchrate / 45;
	CheckRange(spitch, -1.0, 1.0);
	int pitchrect = (int)(spitch * (double)height / 4);
	if (pitchrect >= 0)
		skp->Rectangle(width - border - rwidth, height / 2, width - border, (height / 2) + pitchrect);
	else
		skp->Rectangle(width - border - rwidth, (height / 2) + pitchrect, width - border, height / 2);
	double sroll = fs.rollrate / 45;
	CheckRange(sroll, -1.0, 1.0);
	int rollrect = (int)(sroll * (double)width / 4);
	if (rollrect >= 0)
		skp->Rectangle(width / 2, border, (width / 2) + rollrect, border + rwidth);
	else
		skp->Rectangle((width / 2) + rollrect, border, width / 2, border + rwidth);
	double syaw = fs.yawrate / 45;
	CheckRange(syaw, -1.0, 1.0);
	int yawrect = (int)(syaw * (double)width / 4);
	if (yawrect >= 0)
		skp->Rectangle(width / 2, height - border - rwidth, (width / 2) + yawrect, height - border);
	else
		skp->Rectangle((width / 2) + yawrect, height - border - rwidth, width / 2, height - border);
}