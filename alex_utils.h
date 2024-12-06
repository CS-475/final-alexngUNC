#ifndef alex_utils_DEFINED
#define alex_utils_DEFINED
#include "include/GPixel.h"
#include "include/GPaint.h"

static inline uint32_t div_255(unsigned n) {
	return (n + 128) * 257 >> 16;
}

static inline int relu(int x) {
	return x & ~((x >> 31) & x);
}

static inline int positiveRound(float x) {
	return (int) (x + 0.5f);
}

static inline GPixel makePixelFromPaint(const GPaint& paint) {
	GColor color = paint.getColor();
	unsigned alpha = GRoundToInt(color.a * 255.0f);
	unsigned red = GRoundToInt(color.a * color.r * 255.0f);
	unsigned green = GRoundToInt(color.a * color.g * 255.0f);
	unsigned blue = GRoundToInt(color.a * color.b * 255.0f);
	return GPixel_PackARGB(alpha, red, green, blue);
}

static inline GPixel makePixelFromColor(const GColor& color) {
	unsigned alpha = GRoundToInt(color.a * 255.0f);
	unsigned red = GRoundToInt(color.a * color.r * 255.0f);
	unsigned green = GRoundToInt(color.a * color.g * 255.0f);
	unsigned blue = GRoundToInt(color.a * color.b * 255.0f);
	return GPixel_PackARGB(alpha, red, green, blue);
}

static inline GPixel makePixelFromOpaqueColor(const GColor& color) {
	unsigned red = GRoundToInt(color.a * color.r * 255.0f);
	unsigned green = GRoundToInt(color.a * color.g * 255.0f);
	unsigned blue = GRoundToInt(color.a * color.b * 255.0f);
	return GPixel_PackARGB(255, red, green, blue);
}

static inline GPixel makePixelFromColor2(const GColor& color) {
	unsigned alpha = positiveRound(color.a * 255.0f);
	unsigned red = positiveRound(color.a * color.r * 255.0f);
	unsigned green = positiveRound(color.a * color.g * 255.0f);
	unsigned blue = positiveRound(color.a * color.b * 255.0f);
	return GPixel_PackARGB(alpha, red, green, blue);
}

static inline GPixel makePixelFromOpaqueColor2(const GColor& color) {
	unsigned red = positiveRound(color.r * 255.0f);
	unsigned green = positiveRound(color.g * 255.0f);
	unsigned blue = positiveRound(color.b * 255.0f);
	return GPixel_PackARGB(255, red, green, blue);
}

static inline int clampFloor(float x, float maxBound) {
	return GFloorToInt(std::min(std::max(x, 0.0f), maxBound));
}


#endif

