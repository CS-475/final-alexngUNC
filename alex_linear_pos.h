#ifndef alex_linear_pos_DEFINED
#define alex_linear_pos_DEFINED

#include "include/GShader.h"
#include "include/GMatrix.h"

class LinearPosGradient : public GShader {
private:
	int x;
public:
	LinearPosGradient(GPoint p0, GPoint p1, const GColor colors[], const float pos[], int count) {
		x = 1;
	}

	bool isOpaque() override {
		return false;
	}

	bool setContext(const GMatrix& ctm) override {
		return true;
	}

	void shadeRow(int x, int y, int count, GPixel row[]) override {
		for (int i=0; i<count; i++) {
			row[i] = 0;
		}
	}
};

#endif