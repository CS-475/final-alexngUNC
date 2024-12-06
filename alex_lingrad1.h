#ifndef alex_lingrad1_DEFINED
#define alex_lingrad1_DEFINED

#include "include/GMatrix.h"
#include "include/GShader.h"
#include "alex_utils.h"

class MyLinearGradient1 : public GShader {
private:
	GMatrix m;
	GColor color;
	GMatrix fInverse;
	bool fOpaque;
	GPixel fPixel;
	GPixel fOpaquePixel;

public:
	MyLinearGradient1(GPoint p0, GPoint p1, GColor color) {
		// unit line mapping
		float a = p1.x - p0.x;
		float b = p1.y - p0.y;
		float c = -b;
		m = GMatrix(a, c, p0.x, b, a, p0.y);

		// get inverse for unit mapping
		if (auto inv = m.invert())
			fInverse = *inv;

		// copy color
		this->color = color;

		// calculate opaqueness
		fOpaque = true;
		if (color.a != 1)
			fOpaque = false;

		// get pixels for opaque case
		fOpaquePixel = makePixelFromOpaqueColor2(color);
		fPixel = makePixelFromColor2(color);
	}

	bool isOpaque() override {
		return fOpaque;
	}

	bool setContext(const GMatrix& ctm) override {
	 	if (auto inv = (ctm * m).invert()) {
            fInverse = *inv;
            return true;
        }
        return false;
	}
	
	void shadeRow(int x, int y, int count, GPixel row[]) override {
		GPixel pixel;
		if (fOpaque) {
			pixel = fOpaquePixel;
		} else{
			pixel = fPixel;
		}
		for (int i=0; i<count; i++)
			row[i] = pixel;	
	}

};

#endif