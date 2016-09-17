#include "commons.h"

void getRotMatrix(VECTOR3 arot, MATRIX3* matrix) {
	double tcos = cos(arot.z);
	double tsin = sin(arot.z);
	MATRIX3 z = { 0, 0, 0, 0, 0, 0, 0, 0, 1 };
	z.m11 = z.m22 = tcos;
	z.m12 = -tsin;
	z.m21 = tsin;
	tcos = cos(arot.y);
	tsin = sin(arot.y);
	MATRIX3 y = { 0, 0, 0, 0, 1, 0, 0, 0, 0 };
	y.m11 = y.m33 = tcos;
	y.m13 = tsin;
	y.m31 = -tsin;
	tcos = cos(arot.x);
	tsin = sin(arot.x);
	MATRIX3 x = { 1, 0, 0, 0, 0, 0, 0, 0, 0 };
	x.m22 = x.m33 = tcos;
	x.m32 = tsin;
	x.m23 = -tsin;
	MATRIX3 temp = mul(z, y);
	*matrix = mul(temp, x);
}