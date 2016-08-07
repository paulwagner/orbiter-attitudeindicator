#ifndef _ADI_H_
#define _ADI_H_

#include "orbitersdk.h"
#include "Configuration.h"
#include <gl/gl.h>
#include <gl/glu.h>
#include "MFDCore.h"

class AttitudeReferenceADI;
class Configuration;

class ADI {
public:
	ADI(int x, int y, int width, int height, AttitudeReferenceADI* attref, double cw, double ch, CONFIGURATION& config, MFDSettings* settings);
	~ADI();
	virtual void DrawBall(oapi::Sketchpad* skp, double zoom);

protected:
	oapi::Pen* penWing, *penTurnVec, *penGrad, *penNormal, *penRadial, *penPerpendicular, *penTarget, *penIndicators;
	oapi::Brush* brushWing, *brushTurnVec, *brushGrad, *brushNormal, *brushRadial, *brushPerpendicular, *brushTarget, *brushIndicators;

	void DrawWing(oapi::Sketchpad* skp);
	void DrawTurnVector(oapi::Sketchpad* skp);
	void DrawVectors(oapi::Sketchpad* skp);
	void ProjectVector(VECTOR3 vector, double& x, double& y, double &phi);
	void DrawRateIndicators(oapi::Sketchpad* skp);
	void DrawDirectionArrow(oapi::Sketchpad* skp, oapi::IVECTOR2 v);

private:
	void GetOpenGLRotMatrix(double* m);

	int x, y;
	int width, height;
	AttitudeReferenceADI* attref;
	MFDSettings* settings;
	double cw, ch;
	double diameter;

	HDC hDC;
	HGLRC hRC;
	HBITMAP hBMP;
	HBITMAP hBMP_old;
	GLUquadric* quad;
	GLuint textureId;
};

#endif