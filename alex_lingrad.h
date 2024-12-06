#ifndef alex_lingrad_DEFINED
#define alex_lingrad_DEFINED

#include "include/GMatrix.h"
#include "include/GShader.h"
#include "alex_utils.h"
#include "alex_tiling.h"

class MyLinearGradient : public GShader {
private:
	GMatrix m;
	std::vector<GColor> colors;
	std::vector<GColor> colorDiffs;
	int colorCount;
	GMatrix fInverse;
	bool fOpaque;
	float fCount;
	GPixel fStartPixel;
	GPixel fEndPixel;
	using ShadeRowProc = void (MyLinearGradient::*)(GPixel*, int, float, float);
	ShadeRowProc shadeRowImpl;

public:
	MyLinearGradient(GPoint p0, GPoint p1, const GColor colors[], int count, GTileMode mode)
	: colorCount(count) 
	{
		fCount = (float) count - 1;
		// unit line mapping
		float a = p1.x - p0.x;
		float b = p1.y - p0.y;
		float c = -b;
		m = GMatrix(a, c, p0.x, b, a, p0.y);

		// get inverse for unit mapping
		if (auto inv = m.invert())
			fInverse = *inv;

		// copy colors
		for (int i=0; i<count; i++) {
			this->colors.push_back(colors[i]);
		}

		// calculate color differences
		for (int i=0; i<count-1; i++)
			colorDiffs.push_back(colors[i+1] - colors[i]);

		// calculate opaqueness
		fOpaque = true;
		for (int i=0; i<colorCount; i++) {
			if (colors[i].a != 1)
				fOpaque = false;
		}

		switch (mode) {
			case GTileMode::kClamp:
				if (fOpaque) {
					this->shadeRowImpl = &MyLinearGradient::shadeOpaqueRowClamp;
				} else {
					this->shadeRowImpl = &MyLinearGradient::shadeRowWithAlphaClamp;
				}
				break;
			case GTileMode::kRepeat:
				if (fOpaque) {
					this->shadeRowImpl = &MyLinearGradient::shadeOpaqueRowRepeat;
				} else {
					this->shadeRowImpl = &MyLinearGradient::shadeRowWithAlphaRepeat;
				}
				break;
			case GTileMode::kMirror:
				if (fOpaque) {
					this->shadeRowImpl = &MyLinearGradient::shadeOpaqueRowMirror;
				} else {
					this->shadeRowImpl = &MyLinearGradient::shadeRowWithAlphaMirror;
				}
				break;
			default:
				break;
		}

		// get pixels for opaque case
		fStartPixel = makePixelFromColor2(colors[0]);

		// get end pixel to prevent clamping
		fEndPixel = makePixelFromColor2(colors[count - 1]);
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

	void shadeOpaqueRowClamp(GPixel *row, int count, float xCoord, float step) {
		xCoord *= fCount;
		step *= fCount;
		for (int i=0; i<count; i++) {
			if (xCoord < 0) {
				row[i] = fStartPixel;
			} else if (xCoord >= fCount) {
				row[i] = fEndPixel;
			} else {
				int index = GFloorToInt(xCoord);
				float t = (xCoord - index);
				GColor color = colors[index] + colorDiffs[index] * t;
				row[i] = makePixelFromOpaqueColor2(color);
			}
			xCoord += step;
		}
	}

	void shadeRowWithAlphaClamp(GPixel *row, int count, float xCoord, float step) {
		xCoord *= fCount;
		step *= fCount;
		for (int i=0; i<count; i++) {
			if (xCoord < 0) {
				row[i] = fStartPixel;
			} else if (xCoord >= fCount) {
				row[i] = fEndPixel;
			} else {
				int index = GFloorToInt(xCoord);
				float t = (xCoord - index);
				GColor color = colors[index] + colorDiffs[index] * t;
				row[i] = makePixelFromColor2(color);
			}
			xCoord += step;
		}
	}

	void shadeOpaqueRowRepeat(GPixel *row, int count, float xCoord, float step) {
		// float xdiv = 1.0f / fCount;
		// for (int i=0; i<count; i++) {
		// 	float x = xCoord * xdiv;
		// 	int index = GFloorToInt(x);
		// 	float t = (xCoord - index)
		// 	GColor color = (1 - t) * colors[index] + t * colors[index + 1];
		// 	row[i] = makePixelFromOpaqueColor(color);
		// 	xCoord += step;
		// }
		// UNUSED
		for (int i=0; i<count; i++)
			row[i] = 0;
	}

	void shadeRowWithAlphaRepeat(GPixel *row, int count, float xCoord, float step) {
		// float xdiv = 1.0f / fCount;
		// for (int i=0; i<count; i++) {
		// 	float x = xCoord * xdiv;
		// 	int index = GFloorToInt(x);
		// 	float t = (xCoord - index);
		// 	GColor color = (1 - t) * colors[index] + t * colors[index + 1];
		// 	row[i] = makePixelFromColor(color);
		// 	xCoord += step;
		// }
		// UNUSED
		for (int i=0; i<count; i++)
			row[i] = 0;
	}

	void shadeOpaqueRowMirror(GPixel *row, int count, float xCoord, float step) {
		for (int i=0; i<count; i++) {
			float x = tileAndMirror(xCoord) * fCount;
			int index = GFloorToInt(x);
			float t = (x - index);
			GColor color = colors[index] + colorDiffs[index] * t;
			row[i] = makePixelFromOpaqueColor2(color);
			xCoord += step;
		}
	}

	void shadeRowWithAlphaMirror(GPixel *row, int count, float xCoord, float step) {
		for (int i=0; i<count; i++) {
			float x = tileAndMirror(xCoord) * fCount;
			int index = GFloorToInt(x);
			float t = (x - index);
			GColor color = colors[index] + colorDiffs[index] * t;
			row[i] = makePixelFromColor2(color);
			xCoord += step;
		}
	}
	
	void shadeRow(int x, int y, int count, GPixel row[]) override {
		// send pts through inv transformation
		float a = fInverse[0];
		float px = a * (x + 0.5f) + fInverse[2] * (y + 0.5f) + fInverse[4];

		// map to unit length line segment
		// float xCoord = px * fCount;
		// float step = a * fCount;
		// (this->*shadeRowImpl)(row, count, xCoord, step);
		(this->*shadeRowImpl)(row, count, px, a);
	}
};

#endif