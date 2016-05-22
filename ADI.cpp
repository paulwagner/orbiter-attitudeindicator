#include "ADI.h"
#include "AttitudeReferenceADI.h"
#include <sstream>
#include "Configuration.h"
#include "imageloader.h"

ADI::ADI(int x, int y, int width, int height, AttitudeReferenceADI* attref, double cw, double ch, CONFIGURATION& config) {
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
	this->attref = attref;
	this->cw = cw;
	this->ch = ch;

	penWing = oapiCreatePen(1, 3, config.wingColor);
	penTurnVec = oapiCreatePen(1, 2, config.turnVecColor);
	penGrad = oapiCreatePen(1, 3, config.progradeColor);
	penNormal = oapiCreatePen(1, 3, config.normalColor);
	penRadial = oapiCreatePen(1, 3, config.radialColor);
	penPerpendicular = oapiCreatePen(1, 3, config.perpendicularColor);
	penTarget = oapiCreatePen(1, 3, config.targetColor);
	penIndicators = oapiCreatePen(1, 2, config.indicatorColor);
	brushWing = oapiCreateBrush(config.wingColor);
	brushTurnVec = oapiCreateBrush(config.turnVecColor);
	brushGrad = oapiCreateBrush(config.progradeColor);
	brushNormal = oapiCreateBrush(config.normalColor);
	brushRadial = oapiCreateBrush(config.radialColor);
	brushPerpendicular = oapiCreateBrush(config.perpendicularColor);
	brushTarget = oapiCreateBrush(config.targetColor);
	brushIndicators = oapiCreateBrush(config.indicatorColor);

	drawPrograde = config.startPrograde;
	drawNormal = config.startNormal;
	drawRadial = config.startRadial;
	drawPerpendicular = config.startPerpendicular;
	turnVectorMode = config.startTurnVectorMode;

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
	delete penWing;
	delete penTurnVec;
	delete penGrad;
	delete penNormal;
	delete penRadial;
	delete penPerpendicular;
	delete penTarget;
	delete penIndicators;
	delete brushWing;
	delete brushTurnVec;
	delete brushGrad;
	delete brushNormal;
	delete brushRadial;
	delete brushPerpendicular;
	delete brushTarget;
	delete brushIndicators;
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
	HDC	hDC = skp->GetDC();
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
	BitBlt (hDC, x, y, width, height, this->hDC, 0, 0, SRCCOPY);

	DrawVectors(skp);
	if (turnVectorMode == 1)
		DrawRateIndicators(skp);
	if (turnVectorMode == 2)
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
	skp->SetBrush(brushWing);
	skp->MoveTo(width / 2 - (int)(cw * 4), height / 2);
	skp->LineTo(width / 2 - (int)(cw * 3 / 2), height / 2);
	skp->LineTo(width / 2, height / 2 + (int)ch);
	skp->LineTo(width / 2 + (int)(cw * 3 / 2), height / 2);
	skp->LineTo(width / 2 + (int)(cw * 4), height / 2);
	skp->Rectangle(width / 2 + 1, height / 2 + 1, width / 2 - 1, height / 2 - 1);
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
	int s1 = (int)(cw * 2 / 3); // Radial size of arrow
	double s2 = (cw/3) * RAD; // Radial width of arrow
	v.x -= (int)(width / 2);
	v.y -= (int)(height / 2);
	// To polar coordinates
	double phi = atan2(v.y, v.x);
	double r = diameter / 2;
	// Back to sketchpad coordinates
	oapi::IVECTOR2 pts[4];
	pts[0].x = (int)(r * cos(phi)); pts[0].y = (int)(r * sin(phi));
	pts[1].x = (int)((r - 3*s1) * cos(phi - s2)); pts[1].y = (int)((r - 3*s1) * sin(phi - s2));
	pts[2].x = (int)((r - 2*s1) * cos(phi)); pts[2].y = (int)((r - 2*s1) * sin(phi));
	pts[3].x = (int)((r - 3*s1) * cos(phi + s2)); pts[3].y = (int)((r - 3*s1)* sin(phi + s2));
	for (int i = 0; i < 4; i++) {
		pts[i].x += (int)(width / 2);
		pts[i].y += (int)(height / 2);
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
	VECTOR3 pgd, nml, rad, pep, tgt, dock;
	int frm = attref->GetMode();
	double phi;
	double x, y;
	int ix, iy;
	double phiF = 70; // max marker view angle

	// Calculate direction vectors
	if (frm == 4 && fs.navType == TRANSMITTER_IDS)
		attref->GetDockingPortDirection(dock);
	if (frm == 2)
		attref->GetOrbitalSpeedDirection(pgd, nml, rad, pep); // Orbital relative vector in OV/OM
	else if (frm == 3)
		attref->GetAirspeedDirection(pgd, nml, rad, pep); // Surface relative vector in LH/LN
	else if (frm == 4 && fs.hasNavTarget)
		attref->GetTargetDirections(tgt, pgd); // Target relative vector in NAV
	
	if (frm <= 1)
		return; // No markers in ECL and EQU

	// Prograde
	if (drawPrograde && (frm != 4 || (fs.hasNavTarget && (fs.navType == TRANSMITTER_IDS || fs.navType == TRANSMITTER_XPDR || fs.navType == TRANSMITTER_VTOL))) && isnormal(length(pgd))) {
		double d = sin(45 * RAD);
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
			skp->Ellipse(ix + (int)cw, iy + (int)ch, ix - (int)cw, iy - (int)ch);
			skp->Line(ix, iy - (int)ch, ix, iy - 2 * (int)ch);
			skp->Line(ix + (int)cw, iy, ix + 2 * (int)cw, iy);
			skp->Line(ix - (int)cw, iy, ix - 2 * (int)cw, iy);
		}
		// Retrograde
		ProjectVector(-pgd, x, y, phi);
		ix = (int)x, iy = (int)y;
		if (abs(phi) <= phiF) {
			skp->SetPen(penGrad);
			skp->SetBrush(NULL);
			skp->Ellipse(ix + (int)cw, iy + (int)ch, ix - (int)cw, iy - (int)ch);
			skp->Line(ix, iy - (int)ch, ix, iy - 2 * (int)ch);
			skp->Line(ix + (int)cw, iy, ix + 2 * (int)cw, iy);
			skp->Line(ix - (int)cw, iy, ix - 2 * (int)cw, iy);
			skp->Line(ix - (int)(cw*d), iy - (int)(ch*d), ix + (int)(cw*d), iy + (int)(ch*d));
			skp->Line(ix - (int)(cw*d), iy + (int)(ch*d), ix + (int)(cw*d), iy - (int)(ch*d));
		} else if (!pgdVisible){
			// No marker visible, draw direction arrow to prograde
			skp->SetPen(penGrad);
			skp->SetBrush(brushGrad);
			DrawDirectionArrow(skp, pgdDir);
		}

	}

	if (frm == 4){
		// Target marker
		if (fs.hasNavTarget){
			int tx = (int)(cw / 2);
			int ty = (int)(ch / 2);
			double d = sin(45 * RAD);
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
				DrawArc(skp, ix + tx, iy - ty, (int)(cw), 0, 90);
				DrawArc(skp, ix + tx, iy + ty, (int)(cw), 90, 180);
				DrawArc(skp, ix - tx, iy + ty, (int)(cw), 180, 270);
				DrawArc(skp, ix - tx, iy - ty, (int)(cw), 270, 360);
			}
			// Anti-target
			ProjectVector(-tgt, x, y, phi);
			ix = (int)x, iy = (int)y;
			if (abs(phi) <= phiF) {
				skp->SetPen(penTarget);
				skp->SetBrush(brushTarget);
				skp->Ellipse(ix - tx / 2, iy - ty / 2, ix + tx / 2, iy + ty / 2);
				skp->SetBrush(NULL);
				skp->Line(ix, iy + (int)ch, ix, iy + 2 * (int)ch);
				skp->Line(ix + (int)(cw*d), iy - (int)(ch*d), ix + 2 * (int)(cw*d), iy - 2 * (int)(ch*d));
				skp->Line(ix - (int)(cw*d), iy - (int)(ch*d), ix - 2 * (int)(cw*d), iy - 2 * (int)(ch*d));
			}
			else if (!tgtVisible){
				// No marker visible, draw direction arrow to target
				skp->SetPen(penTarget);
				skp->SetBrush(brushTarget);
				DrawDirectionArrow(skp, tgtDir);
			}
		}
	
		// Draw course marker
		if (fs.navType == TRANSMITTER_ILS || fs.navType == TRANSMITTER_NONE || fs.navType == TRANSMITTER_VOR) {
			int tx = (int)(cw);
			int ty = (int)(ch);
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
				skp->Line(ix, iy - (int)(ch * 2 / 3), ix, iy + (int)(ch / 3));
				skp->Line(ix - (int)(cw*dx*crsScale), iy + (int)(ch*dy*crsScale), ix + (int)(cw*dx*crsScale), iy + (int)(ch*dy*crsScale));
				skp->Line(ix - (int)(cw*dx*crsScale), iy + (int)(ch*dy*crsScale), ix, iy - (int)(ch*crsScale));
				skp->Line(ix, iy - (int)(ch*crsScale), ix + (int)(cw*dx*crsScale), iy + (int)(ch*dy*crsScale));

			}
		}
		// Draw docking port
		if (fs.navType == TRANSMITTER_IDS) {
			int tx = (int)(cw);
			int ty = (int)(ch);
			ProjectVector(dock, x, y, phi);
			ix = (int)x, iy = (int)y;
			if (abs(phi) <= phiF) {
				skp->SetPen(penWing);
				skp->SetBrush(NULL);
				skp->Ellipse(ix - tx / 2, iy - ty / 2, ix + tx / 2, iy + ty / 2);
			}
		}

		return; // No normal/radial markers in NAV mode	
	}


	// Normal
	if (drawNormal && isnormal(length(pgd))){
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
			skp->Line(ix - (int)(cw*dx*nmlScale), iy + (int)(ch*dy*nmlScale), ix + (int)(cw*dx*nmlScale), iy + (int)(ch*dy*nmlScale));
			skp->Line(ix - (int)(cw*dx*nmlScale), iy + (int)(ch*dy*nmlScale), ix, iy - (int)(ch*nmlScale));
			skp->Line(ix, iy - (int)(ch*nmlScale), ix + (int)(cw*dx*nmlScale), iy + (int)(ch*dy*nmlScale));
		}
		// Anti-Normal
		ProjectVector(-nml, x, y, phi);
		ix = (int)x, iy = (int)y;
		if (abs(phi) <= phiF) {
			skp->SetBrush(brushNormal);
			skp->SetPen(penNormal);
			skp->Ellipse(ix - 1, iy - 1, ix + 1, iy + 1);
			oapi::IVECTOR2 a1; a1.x = ix - (int)(cw*dx*nmlScale); a1.y = iy - (int)(ch*dy*nmlScale);
			oapi::IVECTOR2 a2; a2.x = ix + (int)(cw*dx*nmlScale); a2.y = iy - (int)(ch*dy*nmlScale);
			oapi::IVECTOR2 a3; a3.x = ix; a3.y = iy + (int)(ch*nmlScale);
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

	// Perpendicular out
	if (drawPerpendicular && isnormal(length(pgd))) {
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
			skp->Ellipse(ix + (int)cw, iy + (int)ch, ix - (int)cw, iy - (int)ch);
			skp->Line(ix + (int)(cw*cd), iy + (int)(ch*sd), ix + (int)(pepScale * cw*cd), iy + (int)(pepScale * ch*sd));
			skp->Line(ix + (int)(cw*cd), iy - (int)(ch*sd), ix + (int)(pepScale * cw*cd), iy - (int)(pepScale * ch*sd));
			skp->Line(ix - (int)(cw*cd), iy + (int)(ch*sd), ix - (int)(pepScale * cw*cd), iy + (int)(pepScale * ch*sd));
			skp->Line(ix - (int)(cw*cd), iy - (int)(ch*sd), ix - (int)(pepScale * cw*cd), iy - (int)(pepScale * ch*sd));
		}
		// Perpendicular in
		ProjectVector(-pep, x, y, phi);
		ix = (int)x, iy = (int)y;
		if (abs(phi) <= phiF) {
			skp->SetPen(penPerpendicular);
			skp->SetBrush(brushPerpendicular);
			skp->Ellipse(ix - 1, iy - 1, ix + 1, iy + 1);
			skp->SetBrush(NULL);
			skp->Ellipse(ix + (int)cw, iy + (int)ch, ix - (int)cw, iy - (int)ch);
			skp->Line(ix + (int)(cw*cd), iy + (int)(ch*sd), ix + (int)((2 - pepScale) * cw*cd), iy + (int)((2 - pepScale) * ch*sd));
			skp->Line(ix + (int)(cw*cd), iy - (int)(ch*sd), ix + (int)((2 - pepScale) * cw*cd), iy - (int)((2 - pepScale) * ch*sd));
			skp->Line(ix - (int)(cw*cd), iy + (int)(ch*sd), ix - (int)((2 - pepScale) * cw*cd), iy + (int)((2 - pepScale) * ch*sd));
			skp->Line(ix - (int)(cw*cd), iy - (int)(ch*sd), ix - (int)((2 - pepScale) * cw*cd), iy - (int)((2 - pepScale) * ch*sd));
		} else if (!pepVisible){
			// No marker visible, draw direction arrow to perpendicular
			skp->SetPen(penPerpendicular);
			skp->SetBrush(brushPerpendicular);
			DrawDirectionArrow(skp, pepDir);
		}
	}

	// Radial out
	if (drawRadial && isnormal(length(pgd))) {
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
			// TODO: Think of another marker symbol for radial
			skp->Rectangle(ix + (int)cw, iy + (int)ch, ix - (int)cw, iy - (int)ch);
			skp->Line(ix, iy + (int)(ch), ix, iy + (int)(2 * ch));
			skp->Line(ix, iy - (int)(ch), ix, iy - (int)(2 * ch));
			skp->Line(ix + (int)(cw), iy, ix + (int)(2*ch), iy);
			skp->Line(ix - (int)(cw), iy, ix - (int)(2 * ch), iy);
		}
		// Radial in
		ProjectVector(-rad, x, y, phi);
		ix = (int)x, iy = (int)y;
		if (abs(phi) <= phiF) {
			skp->SetPen(penRadial);
			skp->SetBrush(brushRadial);
			skp->Ellipse(ix - 1, iy - 1, ix + 1, iy + 1);
			skp->SetBrush(NULL);
			skp->Rectangle(ix + (int)cw, iy + (int)ch, ix - (int)cw, iy - (int)ch);
			skp->Line(ix, iy + (int)(ch), ix, iy + (int)(ch / 2));
			skp->Line(ix, iy - (int)(ch), ix, iy - (int)(ch / 2));
			skp->Line(ix + (int)(cw), iy, ix + (int)(ch / 2), iy);
			skp->Line(ix - (int)(cw), iy, ix - (int)(ch / 2), iy);
		} else if (!radVisible){
			// No marker visible, draw direction arrow to radial
			skp->SetPen(penRadial);
			skp->SetBrush(brushRadial);
			DrawDirectionArrow(skp, radDir);
		}
	}

}

void ADI::DrawRateIndicators(oapi::Sketchpad* skp) {
	skp->SetPen(penIndicators);
	skp->SetBrush(NULL);
	int border = (int)(ch*2/3);
	int rwidth = (int)(ch*1.5);
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
	skp->Rectangle(width - border - rwidth, height / 2, width - border, (height / 2) + pitchrect);
	double sroll = fs.rollrate / 45;
	CheckRange(sroll, -1.0, 1.0);
	int rollrect = (int)(sroll * (double)width / 4);
	skp->Rectangle(width / 2, border, (width / 2) + rollrect, border + rwidth);
	double syaw = fs.yawrate / 45;
	CheckRange(syaw, -1.0, 1.0);
	int yawrect = (int)(syaw * (double)width / 4);
	skp->Rectangle(width / 2, height - border, (width / 2) + yawrect, height - border - rwidth);
}

template<class T>
T ADI::CheckRange(T &Var, const T &Min, const T &Max) {
	T Diff = 0;
	if (Var < Min) {
		Diff = Var - Min;
		Var = Min;
	} else if (Var > Max) {
		Diff = Var - Max;
		Var = Max;
	}
	return Diff;
}

template <typename T>
int ADI::sgn(T val) {
	return (T(0) < val) - (val < T(0));
}