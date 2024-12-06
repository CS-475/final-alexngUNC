#ifndef alex_tri_color_shader_DEFINED
#define alex_tri_color_shader_DEFINED

#include "alex_matrix_helpers.h"

class TriColorShader : public GShader {
	GMatrix fLocalMatrix;
	GMatrix fInverse;
	const GColor c0;
	const GColor c1;
	const GColor c2;
	GColor colorDiff1;
	GColor colorDiff2;
	GPixel pixel0;
	GPixel pixel1;
	GPixel pixel2;
	using ShadeRowProc = void (TriColorShader::*)(GPixel*, int, GColor, GColor);
	ShadeRowProc shadeRowImpl;
public:
	TriColorShader(GPoint p0, GPoint p1, GPoint p2, GColor c0, GColor c1, GColor c2)
	: c0(c0), c1(c1), c2(c2)
	{
		// pixels for opaqueness
		pixel0 = makePixelFromColor2(c0);
		pixel1 = makePixelFromColor2(c1);
		pixel2 = makePixelFromColor2(c2);

		if (isOpaque())
			shadeRowImpl = &TriColorShader::shadeRowOpaque;
		else
			shadeRowImpl = &TriColorShader::shadeRowAlpha;

		// get inverse for unit mapping
		fLocalMatrix = computeBases(p0, p1, p2);
		
		// color differences
		colorDiff1 = c1 - c0;
		colorDiff2 = c2 - c0;
	}

	bool isOpaque() override {
		return GPixel_GetA(pixel0) == 0xFF && 
			   GPixel_GetA(pixel1) == 0xFF &&
			   GPixel_GetA(pixel2) == 0xFF;
	}

	bool setContext(const GMatrix& ctm) override {
	 	if (auto inv = (ctm * fLocalMatrix).invert()) {
            fInverse = *inv;
            return true;
        }
        return false;
	}

	void shadeRowOpaque(GPixel row[], int count, GColor color, GColor colorDelta) {
		for (int i=0; i<count; i++) {
			row[i] = makePixelFromOpaqueColor2(color);
			color += colorDelta;
		}
	}

	void shadeRowAlpha(GPixel row[], int count, GColor color, GColor colorDelta) {
		for (int i = 0; i < count; ++i) {
			row[i] = makePixelFromColor2(color);
    		color += colorDelta;
		}
	}

	void shadeRow(int x, int y, int count, GPixel row[]) override {
		float a = fInverse[0];
		float b = fInverse[1];
		float d = fInverse[3];
		float centerX = x + 0.5f;
		float centerY = y + 0.5f;
		float px = a * centerX + fInverse[2] * centerY + fInverse[4];
		float py = b * centerX + d * centerY + fInverse[5];
		GColor color = px * colorDiff1 + py * colorDiff2 + c0;
		GColor colorDelta = a * colorDiff1 + b * colorDiff2;
		(this->*shadeRowImpl)(row, count, color, colorDelta);
	}	

};

std::shared_ptr<GShader> CreateTriColorShader(GPoint p0, GPoint p1, GPoint p2, GColor c0, GColor c1, GColor c2) {
	return std::make_shared<TriColorShader>(p0, p1, p2, c0, c1, c2);
}

#endif
