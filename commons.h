#ifndef __COMMONS_ADI_H
#define __COMMONS_ADI_H

#include "Orbitersdk.h"

#ifdef _DEBUG
#define TRACE(X) oapiWriteLog(X);
#else
#define TRACE(X)
#endif

template<class T>
T CheckRange(T &Var, const T &Min, const T &Max) {
	T Diff = 0;
	if (Var < Min) {
		Diff = Var - Min;
		Var = Min;
	}
	else if (Var > Max) {
		Diff = Var - Max;
		Var = Max;
	}
	return Diff;
}

template <typename T>
int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

void getRotMatrix(VECTOR3 arot, MATRIX3* matrix);

#endif