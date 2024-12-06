#ifndef alex_shader_DEFINED
#define alex_shader_DEFINED

#include "include/GShader.h"
#include "include/GBitmap.h"
#include "include/GMatrix.h"
#include "alex_utils.h"
#include "alex_tiling.h"

/*
struct IntPoint {
	int x;
	int y;
};

static inline IntPoint deviceToShader(float x, float y, GMatrix inverse, float width, float height) {
	// transform to shader coord system
	GPoint p = { x, y };
	p = inverse * p;

	// clamp to bitmap dims and floor to int
	int xInt = GFloorToInt(std::min(std::max(p.x, 0.0f), width));
	int yInt = GFloorToInt(std::min(std::max(p.y, 0.0f), height));
	return IntPoint{ xInt, yInt };
}
*/


class MyShader : public GShader {
private:
	const GBitmap fBitmap;
	const GMatrix fLocalMatrix;
	GMatrix fInverse;
	const float fBitmapWidth;
	const float fBitmapHeight;
	const float fActualBitmapHeight;
	const float fActualBitmapWidth;
	using ShadeRowProc = void(MyShader::*)(float, float, float, float, int, GPixel*);
	ShadeRowProc shadeRowImpl;
public:
	MyShader(const GBitmap bm, const GMatrix localMatrix, GTileMode mode) 
		: fBitmap(bm), fLocalMatrix(localMatrix), fBitmapWidth((float) bm.width() - 1), fBitmapHeight((float) bm.height() - 1)
		, fActualBitmapHeight((float) bm.height()) , fActualBitmapWidth((float) bm.width())
	{
		switch (mode) {
			case GTileMode::kClamp:
				this->shadeRowImpl = &MyShader::shadeRowClamp;
				break;
			case GTileMode::kRepeat:
				this->shadeRowImpl = &MyShader::shadeRowRepeat;
				break;
			case GTileMode::kMirror:
				this->shadeRowImpl = &MyShader::shadeRowMirror;
				break;
			default:
				break;
		}
	}
	bool isOpaque() override {
		return fBitmap.isOpaque();
	}
	bool setContext(const GMatrix& ctm) override {
		if (auto inv = (ctm * fLocalMatrix).invert()) {
			fInverse = *inv;
			return true;
		}
		return false;
	}
	void shadeRowClamp(float px, float py, float a, float b, int count, GPixel row[]) {
		float width = fBitmapWidth;
		float height = fBitmapHeight;
		int ix = 0;
		int iy = clampFloor(py, height);
		if (b == 0) {
			for (int i=0; i<count; i++) {
				ix = clampFloor(px, width);
				row[i] = *fBitmap.getAddr(ix, iy);
				px += a;
			}
		} else {
			for (int i=0; i<count; i++) {
				ix = clampFloor(px, width);
				iy = clampFloor(py, height);
				row[i] = *fBitmap.getAddr(ix, iy);
				px += a;
				py += b;
			}
		}
	}

	void shadeRowRepeat(float px, float py, float a, float b, int count, GPixel row[]) {
		float xdiv = 1.0f / fActualBitmapWidth;
		float ydiv = 1.0f / fActualBitmapHeight;
		int ix;
		float temp = py * ydiv;
		temp -= GFloorToInt(temp);
		int iy = (int)(temp*fActualBitmapHeight);
		if (b == 0) {
			for (int i=0; i<count; i++) {
				float x = px * xdiv;
				x -= GFloorToInt(x);
				x *= fActualBitmapWidth;
				ix = (int)(x);
				row[i] = *fBitmap.getAddr(ix, iy);
				px += a;
			}
		} else {
			for (int i=0; i<count; i++) {
				float x = px * xdiv;
				x -= GFloorToInt(x);
				x *= fActualBitmapWidth;
				float y = py * ydiv;
				y -= GFloorToInt(y);
				y *= fActualBitmapHeight;
				ix = (int)(x);
				iy = (int)(y);
				row[i] = *fBitmap.getAddr(ix, iy);
				px += a;
				py += b;
			}
		}
	}

	void shadeRowMirror(float px, float py, float a, float b, int count, GPixel row[]) {
		float ydiv = 1.0f / fActualBitmapHeight;
		float temp = tileAndMirror(py*ydiv)*fActualBitmapHeight;
		int iy = (int)(temp);
		float xdiv = 1.0f / fActualBitmapWidth;
		int ix;
		if (b == 0) {
			for (int i=0; i<count; i++) {
				float x = tileAndMirror(px*xdiv)*fActualBitmapWidth;
				ix = (int) x;
				row[i] = *fBitmap.getAddr(ix, iy);
				px += a;
			}
		} else {
			for (int i=0; i<count; i++) {
				float x = tileAndMirror(px*xdiv)*fActualBitmapWidth;
				float y = tileAndMirror(py*ydiv)*fActualBitmapHeight;
				ix = (int) x;
				iy = (int) y;
				row[i] = *fBitmap.getAddr(ix, iy);
				px += a;
				py += b;
			}
		}
	}

	void shadeRow(int x, int y, int count, GPixel row[]) override {
		float a = fInverse[0];
		float b = fInverse[1];
		float centerX = x + 0.5f;
		float centerY = y + 0.5f;
		float px = a * centerX + fInverse[2] * centerY + fInverse[4];
		float py = b * centerX + fInverse[3] * centerY + fInverse[5];
		(this->*shadeRowImpl)(px, py, a, b, count, row);
	}
};

std::shared_ptr<GShader> GCreateBitmapShader(const GBitmap& bitmap, const GMatrix& localMatrix, GTileMode mode) {
	return  std::make_shared<MyShader>(bitmap, localMatrix, mode);
}

#endif