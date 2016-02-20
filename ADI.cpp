#include "ADI.h"
#include "AttitudeReferenceADI.h"
#include <gl/gl.h>
#include <gl/glu.h>
#include <sstream>

ADI::ADI(int x, int y, int width, int height, AttitudeReferenceADI* attref, double cw, double ch) {
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
	this->attref = attref;
	this->cw = cw;
	this->ch = ch;

	penWing = oapiCreatePen(1, 2, WHITE);
	penTurnVec = oapiCreatePen(1, 2, GREEN);
	penGrad = oapiCreatePen(1, 2, GREEN);
	penNormal = oapiCreatePen(1, 2, RED);
	penRadial = oapiCreatePen(1, 2, BLUE);

	for (int i=0; i < 8; i++)
		NSEW[i] = 0.0;
	GLuint      PixelFormat;  
	BITMAPINFOHEADER BIH;
	int iSize=sizeof(BITMAPINFOHEADER);
	BIH.biSize=iSize;
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
	DWORD code;
	code=GetLastError();
	PixelFormat=ChoosePixelFormat(hDC,&pfd);// now pretend we want a new format
	int ret;
	ret=SetPixelFormat(hDC,PixelFormat,&pfd);
	code=GetLastError();
	hRC=wglCreateContext(hDC);
	ret=wglMakeCurrent(hDC,hRC);					//all standard OpenGL init so far

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset (1.0, 2);

	glViewport(0, 0, width, height);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);           // Panel Background color

	CreateDisplayLists();
}

ADI::~ADI() {
	delete penWing;
	delete penTurnVec;
	delete penGrad;
	delete penNormal;
	delete penRadial;

	wglMakeCurrent(NULL, NULL);	//standard OpenGL release
	wglDeleteContext(hRC);
	hRC=NULL;
	SelectObject(hDC,hBMP_old);//remember to delete DC and bitmap memory we created
	DeleteObject(hBMP);
	DeleteDC(hDC);
}

void ADI::CreateDisplayLists() {
	displayLists[0] = glGenLists(NUM_DLS);

	for (int i = 1; i < NUM_DLS; i++)
		displayLists[i] = displayLists[0] + i;

	float x, y, z, scale;
	int elev, azi, elev2;

	glNewList (displayLists[DL_HEMISPHERE], GL_COMPILE);
		glBegin (GL_TRIANGLE_FAN);
			glVertex3f (0.0f, 1.0f, 0.0f); // top vertex
			elev = 80;
			y = sin (elev*RADf);
			for (azi = 360; azi >= 0; azi -= 10) { // run in a circle at +80 degrees
				x = cos (elev*RADf) * cos (azi*RADf);
				z = cos (elev*RADf) * sin (azi*RADf);
				glVertex3f (x, y, z);
			}
		glEnd();

		for (elev = 70; elev >= 0; elev -= 10) {
			elev2 = elev + 10;
			azi = 0;
			glBegin (GL_QUAD_STRIP);

				y = sin (elev*RADf);
				x = cos (elev*RADf) * cos (azi*RADf);
				z = cos (elev*RADf) * sin (azi*RADf);
				glVertex3f (x, y, z);

				y = sin (elev2*RADf);
				x = cos (elev2*RADf) * cos (azi*RADf);
				z = cos (elev2*RADf) * sin (azi*RADf);
				glVertex3f (x, y, z);

				for (azi = 10; azi <= 360; azi += 10) {
					y = sin (elev*RADf);
					x = cos (elev*RADf) * cos (azi*RADf);
					z = cos (elev*RADf) * sin (azi*RADf);
					glVertex3f (x, y, z);

					y = sin (elev2*RADf);
					x = cos (elev2*RADf) * cos (azi*RADf);
					z = cos (elev2*RADf) * sin (azi*RADf);
					glVertex3f (x, y, z);
				}
			glEnd ();
		}
	glEndList();

	glNewList (displayLists[DL_CIRCLE_XZ], GL_COMPILE);
		glBegin (GL_LINE_LOOP);
			for (azi = 0; azi < 360; azi += 10) {
				x = sin (azi*RADf);
				z = cos (azi*RADf);
				glVertex3f (x, 0.0f, z);
			}
		glEnd();
	glEndList();

	glNewList (displayLists[DL_CIRCLE_XY], GL_COMPILE);
		glBegin (GL_LINE_LOOP);
			for (azi = 0; azi < 360; azi += 10) {
				x = cos (azi*RADf);
				y = sin (azi*RADf);
				glVertex3f (x, y, 0.0f);
			}
		glEnd();
	glEndList();

	glNewList (displayLists[DL_LATITUDE_MAJOR], GL_COMPILE);
		for (int elev = 30; elev < 90; elev += 30) {
			glPushMatrix();
				glTranslatef(0.0f, sin (elev*RADf), 0.0f);
				scale = cos (elev*RADf);
				glScalef (scale, scale, scale);
				glCallList(displayLists[DL_CIRCLE_XZ]);
			glPopMatrix();
		}
	glEndList();

	glNewList (displayLists[DL_LATITUDE_MINOR], GL_COMPILE);
		for (int elev = 10; elev < 90; elev += 10) {
			if (elev%30 != 0) {
				glPushMatrix();
					glTranslatef(0.0f, sin (elev*RADf), 0.0f);
					scale = cos (elev*RADf);
					glScalef (scale, scale, scale);
					glCallList(displayLists[DL_CIRCLE_XZ]);
				glPopMatrix();
			}
		}
	glEndList();

	glNewList (displayLists[DL_LONGITUDE], GL_COMPILE);
		for (int azi = 0; azi < 360; azi += 30) {
			glPushMatrix();
				glRotatef ((float)azi, 0.0f, 1.0f, 0.0f);
				glCallList(displayLists[DL_CIRCLE_XY]);
			glPopMatrix();
		}
	glEndList();

	glNewList (displayLists[DL_BALL], GL_COMPILE);
		glPushMatrix();
			glColor3f (0.0f, 0.63f, 0.65f);				// sky background colour
			glCallList (displayLists[DL_HEMISPHERE]);	//render

			glColor3f (0.6f, 0.6f, 0.97f);				// TODO: sky line colour
			glCallList (displayLists[DL_LATITUDE_MAJOR]);

			glColor3f (0.0f, 0.97f, 0.03f); // horizon line colour
			glCallList (displayLists[DL_CIRCLE_XZ]); //render

			glColor3f (0.5f, 0.5f, 0.5f); // TODO: longitude colour
			glCallList (displayLists[DL_LONGITUDE]);

			glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
			glColor3f (0.38f, 0.25f, 0.15f); // earth background colour
			glCallList (displayLists[DL_HEMISPHERE]);	//render

			glColor3f (1.0f, 0.5f, 0.28f);
			glCallList (displayLists[DL_LATITUDE_MAJOR]);
		glPopMatrix();
	glEndList();
}

void ADI::DrawBall(oapi::Sketchpad* skp, double zoom) {
	HDC	hDC = skp->GetDC();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);             // Clear The Screen And The Depth Buffer        

	double zoomd = 1.0 / zoom;
	double aspect = (double)width / (double)height;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if (aspect > 1)  // wider than tall
		glOrtho (-zoomd * aspect, zoomd * aspect, -zoomd, zoomd, 2.0, 4.0);
	else
		glOrtho (-zoomd, zoomd, -zoomd / aspect, zoomd / aspect, 2.0, 4.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -3.0f);
	double m[16];
	GetOpenGLRotMatrix(m);
	glMultMatrixd(m);

	glCallList (displayLists[DL_BALL]);

	glEnable(GL_LINE_STIPPLE);
	glLineStipple(5, 0x5555);

	glColor3f (0.6f, 0.6f, 0.97f);				// TODO: sky line colour
	glCallList (displayLists[DL_LATITUDE_MINOR]);

	glPushMatrix();
		glColor3f (1.0f, 0.5f, 0.28f);
		glScalef (-1.0f, -1.0f, -1.0f);
		glCallList (displayLists[DL_LATITUDE_MINOR]);
	glPopMatrix();

	glDisable(GL_LINE_STIPPLE);

	glFlush();
	glFinish();
	BitBlt (hDC, x, y, width, height, this->hDC, 0, 0, SRCCOPY);

	DrawSurfaceText(skp);
	DrawWing(skp);
	DrawTurnVector(skp);
	//DrawPrograde(skp);
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

void ADI::DrawSurfaceText(oapi::Sketchpad* skp) {
	GLdouble model[16];
	GLdouble proj[16];
	GLint view[4];
	GLdouble z;

	glGetDoublev(GL_MODELVIEW_MATRIX, model);
	glGetDoublev(GL_PROJECTION_MATRIX, proj);
	glGetIntegerv(GL_VIEWPORT, view);
	gluProject(0.0, 0.0, 1.0, model, proj, view, NSEW + 0, NSEW + 1, &z);
	gluProject(0.0, 0.0, -1.0, model, proj, view, NSEW + 2, NSEW + 3, &z);
	gluProject(1.0, 0.0, 0.0, model, proj, view, NSEW + 4, NSEW + 5, &z);
	gluProject(-1.0, 0.0, 0.0, model, proj, view, NSEW + 6, NSEW + 7, &z);


	for (int i = 0; i < 8; i += 2)
		CheckRange(NSEW[i], (double)0, (double)width);
	for (int i = 1; i < 8; i += 2) {
		CheckRange(NSEW[i], (double)0, (double)height);
		NSEW[i] = height - NSEW[i]; // invert y coords
	}

	skp->SetTextColor(WHITE);

	FLIGHTSTATUS fs = attref->GetFlightStatus();
	if (fs.heading > 267 || fs.heading < 93 || abs(fs.pitch) > 80) {
		skp->Text((int)(x + NSEW[0] - cw / 2),
			(int)(y + NSEW[1] - ch / 2), "N", 1);
	}

	if (fs.heading > 87 && fs.heading < 273 || abs(fs.pitch) > 80) {
		skp->Text((int)(x + NSEW[2] - cw / 2),
			(int)(y + NSEW[3] - ch / 2), "S", 1);
	}

	if (fs.heading > 357 || fs.heading < 183 || abs(fs.pitch) > 80) {
		skp->Text((int)(x + NSEW[4] - cw / 2),
			(int)(y + NSEW[5] - ch / 2), "E", 1);
	}

	if (fs.heading > 177 || fs.heading < 3 || abs(fs.pitch) > 80) {
		skp->Text((int)(x + NSEW[6] - cw / 2),
			(int)(y + NSEW[7] - ch / 2), "W", 1);
	}
}


void ADI::DrawWing(oapi::Sketchpad* skp) {
	skp->SetPen(penWing);
	skp->MoveTo(width / 2 - (int)(cw * 4), height / 3);
	skp->LineTo(width / 2 - (int)(cw * 3 / 2), height / 3);
	skp->LineTo(width / 2, height / 3 + (int)ch);
	skp->LineTo(width / 2 + (int)(cw * 3 / 2), height / 3);
	skp->LineTo(width / 2 + (int)(cw * 4), height / 3);
	//skp->MoveTo(width / 2 - 40, height / 2);
	//skp->LineTo(width / 2 - 15, height / 2);
	//skp->LineTo(width / 2, height / 2 + 10);
	//skp->LineTo(width / 2 + 15, height / 2);
	//skp->LineTo(width / 2 + 40, height / 2);
}

void ADI::DrawTurnVector(oapi::Sketchpad* skp) {
	// Yaw = -heading?
	GLdouble model[16];
	GLdouble proj[16];
	GLint view[4];
	GLdouble z;

	GLdouble fp[3];
	float fpPitch, fpHeading;
	FLIGHTSTATUS fs = attref->GetFlightStatus();

	glGetDoublev(GL_MODELVIEW_MATRIX, model);
	glGetDoublev(GL_PROJECTION_MATRIX, proj);
	glGetIntegerv(GL_VIEWPORT, view);

	fpPitch = (float)(fs.pitch + fs.pitchrate * 3);
	fpHeading = (float)(fs.yaw + fs.yawrate * 3);

	fp[0] = cos(fpPitch*RADf) * sin(fpHeading*RADf);
	fp[1] = sin(fpPitch*RADf);
	fp[2] = cos(fpPitch*RADf) * cos(fpHeading*RADf);

	double pitchProj, yawProj;
	gluProject(fp[0], fp[1], fp[2], model, proj, view, &yawProj, &pitchProj, &z);

	CheckRange(yawProj, (double)0, (double)width);
	CheckRange(pitchProj, (double)0, (double)height);
	pitchProj = height - pitchProj; // invert y coords
	double roll = abs(fs.rollrate*RADf); // Between 0 and 1

	// Draw vector
	skp->SetPen(penTurnVec);
	skp->MoveTo(width / 2, height / 2);
	//int x = int((float)turnVector[0] + 0.5);
	//int y = int((float)turnVector[1] + 0.5);
	skp->LineTo(int(yawProj+0.5), int(pitchProj+0.5));
	int offset = int((roll*min(width, height) / 2) + 0.5);
	skp->Ellipse(width / 2 - offset, height / 2 - offset, width / 2 + offset, height / 2 + offset);
}

void ADI::DrawPrograde(oapi::Sketchpad* skp) {
	VECTOR3 rVel, rPos, gPos;
	VESSEL *v = oapiGetFocusInterface();
	OBJHANDLE rBody = v->GetGravityRef();	// Get current reference body (the celestial body currently being orbited) 

	v->GetGlobalPos(gPos);		// Get position of vessel in global frame.
	v->GetRelativePos(rBody, rPos);		// Get position of vessel relative to reference body.
	v->GetRelativeVel(rBody, rVel);		// Get velocity of vessel relative to reference body.
	VECTOR3 horizon = crossp(rVel, rPos);	// Use cross-product of 'rVel' and 'rPos' to find horizon reference.

	VECTOR3 vPrograde;
	v->Global2Local((rVel + gPos), vPrograde); // Get prograde vector in local coordinates.
	//v->Global2Local(rVel, vPrograde); // Get prograde vector in local coordinates.
	//double len = sqrt(vPrograde.x*vPrograde.x + vPrograde.y*vPrograde.y + vPrograde.z*vPrograde.z);
	//vPrograde.x /= len;
	//vPrograde.y /= len;
	//vPrograde.z /= len;

	VECTOR3 vNormal = { (vPrograde.x), (vPrograde.y*cos(RAD*-90) - vPrograde.z*sin(RAD*-90)), (vPrograde.y*sin(RAD*-90) + vPrograde.z*cos(RAD*-90)) }; // Rotate prograde vector by -90 degrees to find normal vector

	skp->SetTextColor(WHITE);
	std::ostringstream sstream;
	sstream << vPrograde.x << "," << vPrograde.y << "," << vPrograde.z;
	std::string str = sstream.str();
	skp->Text(20,10,str.c_str(),str.length());

	GLdouble model[16];
	GLdouble proj[16];
	GLint view[4];
	GLdouble z;

	GLdouble fp[3];

	glGetDoublev(GL_MODELVIEW_MATRIX, model);
	glGetDoublev(GL_PROJECTION_MATRIX, proj);
	glGetIntegerv(GL_VIEWPORT, view);

	/*
	fpPitch = (float)(pitch + pitchrate * 3);
	fpHeading = (float)(-yaw + yawrate * 3);

	fp[0] = cos(fpPitch*RADf) * sin(fpHeading*RADf);
	fp[1] = sin(fpPitch*RADf);
	fp[2] = cos(fpPitch*RADf) * cos(fpHeading*RADf);
	*/
	fp[0] = vPrograde.x;
	fp[1] = vPrograde.y;
	fp[2] = vPrograde.z;

	double x, y;
	gluProject(fp[0], fp[1], fp[2], model, proj, view, &x, &y, &z);

	CheckRange(x, (double)0, (double)width);
	CheckRange(y, (double)0, (double)height);
	y = height - y; // invert y coords

	std::ostringstream sstream2;
	sstream2 << x << "," << y << "," << z;
	std::string strs = sstream2.str();
	skp->Text(20, 50, strs.c_str(), strs.length());

}

template<class T>
void ADI::CheckRange(T &Var, const T &Min, const T &Max)
{
	if (Var < Min)
		Var = Min;
	else
		if (Var > Max)
			Var = Max;
}