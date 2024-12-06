#ifndef alex_tree_shader_DEFINED
#define alex_tree_shader_DEFINED

#include "alex_utils.h"

class TreeShader : public GShader {
	const GMatrix fLocalMatrix;
	const GPixel fTrunk, fLeaves;
	GMatrix fInverse;
	float fBitmapWidth;
	float fBitmapHeight;
public:
	TreeShader(GPixel trunk, GPixel leaves, float width, float height)
		: fLocalMatrix(GMatrix())
		, fTrunk(trunk)
		, fLeaves(leaves)
		, fBitmapWidth(width)
		, fBitmapHeight(height)
	{}

	bool isOpaque() override {
		return GPixel_GetA(fTrunk) == 0xFF && GPixel_GetA(fLeaves) == 0xFF;
	}

	bool setContext(const GMatrix& ctm) override {
	 	if (auto inv = (ctm * fLocalMatrix).invert()) {
            fInverse = *inv;
            return true;
        }
        return false;
	}

	void shadeRow(int x, int y, int count, GPixel row[]) override {
		float a = fInverse[0];
		float b = fInverse[1];
		float centerX = x + 0.5f;
		float centerY = y + 0.5f;
		float px = a * centerX + fInverse[2] * centerY + fInverse[4];
		float py = b * centerX + fInverse[3] * centerY + fInverse[5];
		float width = fBitmapWidth;
		float height = fBitmapHeight;
		int ix = 0;
		int iy = 0;
		if (b == 0) {
			for (int i=0; i<count; i++) {
				ix = clampFloor(px, width);
				iy = clampFloor(py, height);
				if (iy <= 100)
					row[i] = fLeaves;
				else
					row[i] = fTrunk;
				px += a;
			}
		} else {
			for (int i=0; i<count; i++) {
				ix = clampFloor(px, width);
				iy = clampFloor(py, height);
				if (iy <= 100)
					row[i] = fLeaves;
				else
					row[i] = fTrunk;

				px += a;
				py += b;
			}
		}
	}	

};

std::shared_ptr<GShader> CreateTreeShader(GPixel trunk, GPixel leaf, float width, float height) {
	return  std::make_shared<TreeShader>(trunk, leaf, width, height);
}

#endif