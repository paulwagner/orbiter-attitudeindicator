#ifndef _ADI_H_
#define _ADI_H_

#include "orbitersdk.h"

#define DL_HEMISPHERE		0
#define DL_CIRCLE_XZ		1
#define DL_CIRCLE_XY		2
#define DL_LATITUDE_MAJOR	3
#define DL_LATITUDE_MINOR	4
#define DL_LONGITUDE		5
#define DL_BALL				6
#define NUM_DLS				7

const float RADf = (float)RAD;

#define WHITE RGB(255,255,255)
#define RED RGB(255,0,0)
#define GREEN RGB(0,255,0)
#define BLUE RGB(0,0,255)
#define PROGRADE RGB(221,255,0)
#define NORMAL RGB(235,11,255)
#define RADIAL RGB(9,254,239)

class AttitudeReferenceADI;

class ADI {
public:
	ADI(int x, int y, int width, int height, AttitudeReferenceADI* attref, double cw, double ch);
	~ADI();
	virtual void DrawBall(oapi::Sketchpad* skp, double zoom);
protected:
	oapi::Pen* penWing, *penTurnVec, *penGrad, *penNormal, *penRadial;
	oapi::Brush* brushWing, *brushTurnVec, *brushGrad, *brushNormal, *brushRadial;

	void DrawSurfaceText(oapi::Sketchpad* skp);
	void DrawWing(oapi::Sketchpad* skp);
	void DrawTurnVector(oapi::Sketchpad* skp);
	void DrawVectors(oapi::Sketchpad* skp);
	void CalcVectors(VECTOR3 vector, double bank, double& x, double& y, double &alpha, double &beta);

private:
	void CreateDisplayLists();
	template<class T>void CheckRange(T &Var, const T &Min, const T &Max);
	template <typename T> int sgn(T val);
	void GetOpenGLRotMatrix(double* m);

	int x, y;
	int width, height;
	AttitudeReferenceADI* attref;
	double cw, ch;

	//some stuff for OpenGL
	HDC hDC;
	HGLRC hRC;
	HBITMAP hBMP;
	HBITMAP hBMP_old;
	int displayLists[NUM_DLS];

	double NSEW[8];
};

#endif