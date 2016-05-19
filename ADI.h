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

#define TURNVECTORCOUNT 3

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
	void inline TogglePerpendicular(){ drawPerpendicular = !drawPerpendicular; }
	void inline ToggleTurnVector(){ turnVectorMode++; turnVectorMode %= TURNVECTORCOUNT; }
	bool inline GetPrograde(){ return drawPrograde; }
	bool inline GetNormal(){ return drawNormal; }
	bool inline GetRadial(){ return drawRadial; }
	bool inline GetPerpendicular(){ return drawPerpendicular; }
	int inline GetTurnVector(){ return turnVectorMode; }
	void inline SetPrograde(bool prograde){ drawPrograde = prograde; }
	void inline SetNormal(bool normal){ drawNormal = normal; }
	void inline SetRadial(bool radial){ drawRadial = radial; }
	void inline SetPerpendicular(bool perpendicular){ drawPerpendicular = perpendicular; }
	void inline SetTurnVector(int newTurnVectorMode){ turnVectorMode = newTurnVectorMode; }

protected:
	oapi::Pen* penWing, *penTurnVec, *penGrad, *penNormal, *penRadial, *penPerpendicular, *penTarget, *penIndicators;
	oapi::Brush* brushWing, *brushTurnVec, *brushGrad, *brushNormal, *brushRadial, *brushPerpendicular, *brushTarget, *brushIndicators;

	void DrawSurfaceText(oapi::Sketchpad* skp);
	void DrawWing(oapi::Sketchpad* skp);
	void DrawTurnVector(oapi::Sketchpad* skp);
	void DrawVectors(oapi::Sketchpad* skp);
	void ProjectVector(VECTOR3 vector, double& x, double& y, double &phi);
	void DrawRateIndicators(oapi::Sketchpad* skp);
	void DrawDirectionArrow(oapi::Sketchpad* skp, oapi::IVECTOR2 v);

private:
	void CreateDisplayLists();
	template<class T>T CheckRange(T &Var, const T &Min, const T &Max);
	template <typename T> int sgn(T val);
	void GetOpenGLRotMatrix(double* m);

	int x, y;
	int width, height;
	AttitudeReferenceADI* attref;
	double cw, ch;
	double diameter;

	bool drawPrograde;
	bool drawNormal;
	bool drawRadial;
	bool drawPerpendicular;
	// 0 - No turn vectors; 1 - PRIs; 2 - Turn vectors on ball
	int turnVectorMode;

	HDC hDC;
	HGLRC hRC;
	HBITMAP hBMP;
	HBITMAP hBMP_old;
	int displayLists[NUM_DLS];

	double NSEW[8];

};

#endif