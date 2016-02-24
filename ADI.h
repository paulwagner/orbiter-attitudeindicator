#ifndef _ADI_H_
#define _ADI_H_

#include "orbitersdk.h"
#include "Configuration.h"

#define DL_HEMISPHERE		0
#define DL_CIRCLE_XZ		1
#define DL_CIRCLE_XY		2
#define DL_LATITUDE_MAJOR	3
#define DL_LATITUDE_MINOR	4
#define DL_LONGITUDE		5
#define DL_BALL				6
#define NUM_DLS				7

const float RADf = (float)RAD;

class AttitudeReferenceADI;
class Configuration;

class ADI {
public:
	ADI(int x, int y, int width, int height, AttitudeReferenceADI* attref, double cw, double ch, CONFIGURATION& texture);
	~ADI();
	virtual void DrawBall(oapi::Sketchpad* skp, double zoom);
	void inline TogglePrograde(){ drawPrograde = !drawPrograde; }
	void inline ToggleNormal(){ drawNormal = !drawNormal; }
	void inline ToggleRadial(){ drawRadial = !drawRadial; }
	void inline ToggleTurnVector(){ drawTurnVector = !drawTurnVector; }

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

	bool drawPrograde;
	bool drawNormal;
	bool drawRadial;
	bool drawTurnVector;

	//some stuff for OpenGL
	HDC hDC;
	HGLRC hRC;
	HBITMAP hBMP;
	HBITMAP hBMP_old;
	int displayLists[NUM_DLS];

	double NSEW[8];

};

#endif