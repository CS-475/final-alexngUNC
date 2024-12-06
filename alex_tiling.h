#ifndef alex_tiling_DEFINED
#define alex_tiling_DEFINED

/*
bitmap(x, y, w, h) -> floatTile(x, w),
linear_gradient() -> tile(t) t/grad_length
*/
// t = position on x axis
static inline float tileAndMirror(float t) {
	float x = t - 1.0f;
	// verify below x is mirrored value for gradient
	x = x - floorf(x * 0.5f) * 2 - 1;
	return fabs(x);
}

#endif