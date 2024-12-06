#ifndef alex_linear_pos_DEFINED
#define alex_linear_pos_DEFINED

#include "include/GShader.h"

class LinearPosGradient : public GShader {
private:
	int x;
public:
	LinearPosGradient(GPoint p0, GPoint p1, const GColor colors[], const float pos[], int count) {
		x = 1;
	}

};

#endif