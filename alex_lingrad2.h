#ifndef alex_lingrad2_DEFINED
#define alex_lingrad2_DEFINED

#include "include/GMatrix.h"
#include "include/GShader.h"
#include "alex_utils.h"
#include "alex_tiling.h"

class MyLinearGradient2 : public GShader {
private:
	GMatrix m;
	GColor color1;
	GColor color2;
	GColor colorDiff;
	GMatrix fInverse;
	bool fOpaque;
	bool allSameColor;
	bool unitStartEnd;
	GPixel fStartPixel;
	using ShadeRowProc = void (MyLinearGradient2::*)(float, float, int, GPixel*);
	ShadeRowProc shadeRowImpl;

public:
	MyLinearGradient2(GPoint p0, GPoint p1, GColor firstColor, GColor secondColor, GTileMode mode) {
		// unit line mapping
		unitStartEnd = true;
		float a = p1.x - p0.x;
		float b = p1.y - p0.y;
		float c = -b;
		m = GMatrix(a, c, p0.x, b, a, p0.y);

		// get inverse for unit mapping
		if (auto inv = m.invert())
			fInverse = *inv;

		// copy colors
		color1 = firstColor;
		color2 = secondColor;
		allSameColor = false;
		if (color1 == color2)
			allSameColor = true;

		// calculate color differences
		colorDiff = color2 - color1;

		// calculate opaqueness
		fOpaque = true;
		if (color1.a != 1 || color2.a != 1)
			fOpaque = false;

		// get pixels for opaque case
		fStartPixel = makePixelFromColor2(color1);

		switch (mode) {
			case GTileMode::kClamp:
				if (fOpaque) {
					this->shadeRowImpl = &MyLinearGradient2::shadeRowOpaqueClamp;
				} else {
					this->shadeRowImpl = &MyLinearGradient2::shadeRowWithAlphaClamp;
				}
				break;
			case GTileMode::kRepeat:
				if (fOpaque) {
					this->shadeRowImpl = &MyLinearGradient2::shadeRowOpaqueRepeat;
				} else {
					this->shadeRowImpl = &MyLinearGradient2::shadeRowWithAlphaRepeat;
				}
				break;
			case GTileMode::kMirror:
				if (fOpaque) {
					this->shadeRowImpl = &MyLinearGradient2::shadeRowOpaqueMirror;
				} else {
					this->shadeRowImpl = &MyLinearGradient2::shadeRowWithAlphaMirror;
				}
				break;
			default:
				break;
		}
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

	void shadeRowOpaqueClamp(float x, float a, int count, GPixel row[]) {
		// for (int i=0; i<count; i++)
		// 	row[i] = 0;
		//TODO: fix?
		GColor dColor = colorDiff * a;
		GColor color = color1 + colorDiff * x;
		unsigned alpha = 255;
		float red = color.r * 255.0f;
		float green = color.g * 255.0f;
		float blue = color.b * 255.0f;
		float dRed = dColor.r * 255.0f;
		float dGreen = dColor.g * 255.0f;
		float dBlue = dColor.b * 255.0f;
		assert(x >= 0 && x <= 1);
		for (int i=0; i<count; i++) {
			int r = (int) red;
			int g = (int) green;
			int b = (int) blue;	
			row[i] = GPixel_PackARGB(alpha, r, g, b);
			red += dRed;
			green += dGreen;
			blue += dBlue;
		}
	}

	void shadeRowWithAlphaClamp(float x, float a, int count, GPixel row[]) {
		// float alpha = color.a * 255.0f;
		// float red = color.r * 255.0f;
		// float green = color.g * 255.0f;
		// float blue = color.b * 255.0f;
		// float dAlpha = dColor.a * 255.0f;
		// float dRed = dColor.r * 255.0f;
		// float dGreen = dColor.g * 255.0f;
		// float dBlue = dColor.b * 255.0f;
		// if (x < 0)
		// 	x = 0.0f;
		// else if (x > 1)
		// 	x = 1.0f;
		GColor dColor = colorDiff * a;
		GColor color = color1 + colorDiff * x;
		assert(x >= 0 && x <= 1);
		for (int i=0; i<count; i++) {
			int a = (int) (color.a * 255.0f + 0.5f);
			int r = (int) (color.a * color.r * 255.0f + 0.5f);
			int g = (int) (color.a * color.g * 255.0f + 0.5f);
			int b = (int) (color.a * color.b * 255.0f + 0.5f);	
			row[i] = GPixel_PackARGB(a, r, g, b);
			color += dColor;
		}
		// } else {
		// 	for (int i=0; i<count; i++) {
		// 		float clamp = x;
		// 		if (clamp < 0)
		// 			clamp = 0;
		// 		else if (clamp > 1)
		// 			clamp = 1;
		// 		GColor c =  color0 + colorDifference * clampedX;
		// 		int a = (int) (color.a * 255.0f + 0.5f);
		// 		int r = (int) (color.a * color.r * 255.0f + 0.5f);
		// 		int g = (int) (color.a * color.g * 255.0f + 0.5f);
		// 		int b = (int) (color.a * color.b * 255.0f + 0.5f);	
		// 		row[i] = GPixel_PackARGB(a, r, g, b);
		// 		color += dColor;
		// 		x += a;
		// 	}	
		// }
	}

	void shadeRowOpaqueFill(float x, float a, int count, GPixel row[]) {
		GColor dColor = colorDiff * a;
		GColor color = color1 + colorDiff * x;
		for (int i=0; i<count; i++) {
			int r = (int) (color.r * 255.0f + 0.5f);
			int g = (int) (color.g * 255.0f + 0.5f);
			int b = (int) (color.b * 255.0f + 0.5f);
			row[i] = GPixel_PackARGB(255, r, g, b);
			color += dColor;
		}
	}

	void shadeRowAlphaFill(float x, float a, int count, GPixel row[]) {
		GColor dColor = colorDiff * a;
		GColor color = color1 + colorDiff * x;
		for (int i=0; i<count; i++) {
			int r = (int) (color.r * 255.0f + 0.5f);
			int g = (int) (color.g * 255.0f + 0.5f);
			int b = (int) (color.b * 255.0f + 0.5f);
			int alpha = (int) (color.a * 255.0f + 0.5f);
			row[i] = GPixel_PackARGB(alpha, r, g, b);
			color += dColor;
		}
	}

	void shadeRowOpaqueRepeat(float x, float a, int count, GPixel row[]) {
		for (int i=0; i<count; i++) {
			float unit = x - floorf(x);
			GColor color = color1 + colorDiff * unit;
			int r = (int) (color.r * 255.0f + 0.5f);
			int g = (int) (color.g * 255.0f + 0.5f);
			int b = (int) (color.b * 255.0f + 0.5f);
			row[i] = GPixel_PackARGB(255, r, g, b);
			x += a;
		}
	}

	void shadeRowWithAlphaRepeat(float x, float a, int count, GPixel row[]) {
		for (int i=0; i<count; i++) {
			float unit = x - floorf(x);
			GColor color = color1 + colorDiff * unit;
			int r = (int) (color.r * 255.0f + 0.5f);
			int g = (int) (color.g * 255.0f + 0.5f);
			int b = (int) (color.b * 255.0f + 0.5f);
			int alpha = (int) (color.a * 255.0f + 0.5f);
			row[i] = GPixel_PackARGB(alpha, r, g, b);
			x += a;
		}
	}

	void shadeRowOpaqueMirror(float x, float a, int count, GPixel row[]) {
		for (int i=0; i<count; i++) {
			float unit = tileAndMirror(x);
			GColor color = color1 + colorDiff * unit;
			int r = (int) (color.r * 255.0f + 0.5f);
			int g = (int) (color.g * 255.0f + 0.5f);
			int b = (int) (color.b * 255.0f + 0.5f);
			row[i] = GPixel_PackARGB(255, r, g, b);
			x += a;
		}
	}

	void shadeRowWithAlphaMirror(float x, float a, int count, GPixel row[]) {
		for (int i=0; i<count; i++) {
			float unit = tileAndMirror(x);
			GColor color = color1 + colorDiff * unit;
			int r = (int) (color.r * 255.0f + 0.5f);
			int g = (int) (color.g * 255.0f + 0.5f);
			int b = (int) (color.b * 255.0f + 0.5f);
			int alpha = (int) (color.a * 255.0f + 0.5f);
			row[i] = GPixel_PackARGB(alpha, r, g, b);
			x += a;
		}
	}
	
	void shadeRow(int x, int y, int count, GPixel row[]) override {
		if (allSameColor) {
			for (int i=0; i<count; i++)
				row[i] = fStartPixel;
			return;
		}
		float a = fInverse[0];
		float centerX = x + 0.5f;
		float centerY = y + 0.5f;
		float px = a * centerX + fInverse[2] * centerY + fInverse[4];
		unitStartEnd = false;
		float end = px + a*count;
		if (px < 0 || px > 1.0f || end < 0 || end > 1.0f)
			unitStartEnd = false;
		if (unitStartEnd) {
			if (fOpaque) this->shadeRowImpl = &MyLinearGradient2::shadeRowOpaqueFill;
			else this->shadeRowImpl = &MyLinearGradient2::shadeRowAlphaFill;
		}
		(this->*shadeRowImpl)(px, a, count, row);
	}

};

#endif
