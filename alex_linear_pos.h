#ifndef alex_linear_pos_DEFINED
#define alex_linear_pos_DEFINED

#include <unordered_map>
#include "include/GShader.h"
#include "include/GMatrix.h"
#include "alex_utils.h"

class LinearPosGradient : public GShader {
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
	using ShadeRowProc = void (LinearPosGradient::*)(GPixel*, int, float, float);
	ShadeRowProc shadeRowImpl;
	std::vector<float> fPos;
	std::vector<float> indices;
	std::unordered_map<float, int> indexMap;

public:
	LinearPosGradient(GPoint p0, GPoint p1, const GColor colors[], const float pos[], int count)
	: colorCount(count)
	{
		for (int i=0; i<count; i++)
			fPos.push_back(pos[i]);

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
		
		for (int i=0; i<count; i++) {
			indices.push_back(pos[i]);
			indexMap[pos[i]] = i;
		}

		this->shadeRowImpl = &LinearPosGradient::shadeRowWithAlphaClamp;

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

	float getBottomColor(float f) {
		float m = 0.0f;
		for (int i=0; i<indices.size(); i++) {
			if (indices[i] < f) {
				m = indices[i];
			}
		}
		return m;
	}

	int getIndex(float bottomColor) {
		return indexMap[bottomColor];
	}

	void shadeRowWithAlphaClamp(GPixel *row, int count, float xCoord, float step) {
		// xCoord *= fCount;
		// step *= fCount;
		for (int i=0; i<count; i++) {
			if (xCoord < 0) {
				row[i] = fStartPixel;
			} else if (xCoord >= 1) {
				row[i] = fEndPixel;
			} else {
				// int index = GFloorToInt(xCoord);
				// int index = getIndex(xCoord);
				float botColor = getBottomColor(xCoord);
				int index = getIndex(botColor);

				float t = (xCoord - botColor);
				GColor color = colors[index] + colorDiffs[index] * t;
				row[i] = makePixelFromColor2(color);
			}
			xCoord += step;
		}
	}

	void shadeRow(int x, int y, int count, GPixel row[]) override {
		float a = fInverse[0];
		float px = a * (x + 0.5f) + fInverse[2] * (y + 0.5f) + fInverse[4];
		(this->*shadeRowImpl)(row, count, px, a);
	}

};

#endif