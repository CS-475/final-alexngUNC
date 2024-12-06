#ifndef alex_color_matrix_DEFINED
#define alex_color_matrix_DEFINED

#include "include/GShader.h"
#include "include/GFinal.h"
#include "alex_utils.h"

//  *  new_color = [0 4  8 12 16] [orig_color.r]
//  *              [1 5  9 13 17] [orig_color.g]
//  *              [2 6 10 14 18] [orig_color.b]
//  *              [3 7 11 15 19] [orig_color.a]

float clampColor(float c) {
	if (c < 0.0f)
		return 0.0f;
	if (c > 1.0f)
		return 1.0f;
	return c;
}

GColor multiplyColorMatrix(GColorMatrix M, GColor old) {
	GColor color;
	float r = M[0] * old.r + M[4] * old.g + M[8] * old.b + M[12] * old.a + M[16];
	float g = M[1] * old.r + M[5] * old.g + M[9] * old.b + M[13] * old.a + M[17];
	float b = M[2] * old.r + M[6] * old.g + M[10] * old.b + M[14] * old.a + M[18];
	float a = M[3] * old.r + M[7] * old.g + M[11] * old.b + M[15] * old.a + M[19];
	color.r = clampColor(r);
	color.g = clampColor(g);
	color.b = clampColor(b);
	color.a = clampColor(a);
	return color;
}

class MyColorMatrixShader : public GShader {
private:
	GShader* fRealShader;
	GColorMatrix fMatrix;

public:
	MyColorMatrixShader(const GColorMatrix& matrix, GShader* realShader) {
		fMatrix = matrix;
		fRealShader = realShader;
	}

    bool isOpaque() override { return fRealShader->isOpaque(); }

    bool setContext(const GMatrix& ctm) override {
		return fRealShader->setContext(ctm);
    }
    
    void shadeRow(int x, int y, int count, GPixel row[]) override {
		GPixel rowBuffer[count];
		fRealShader->shadeRow(x, y, count, rowBuffer);
		for (int i=0; i<count; i++) {
			GColor oldColor = makeColorFromPixel(rowBuffer[i]);
			GColor color = multiplyColorMatrix(fMatrix, oldColor);
			GPixel pixel = makePixelFromColor(color);
			row[i] = pixel;
			// row[i] = rowBuffer[i];
		}
    }
};

#endif 