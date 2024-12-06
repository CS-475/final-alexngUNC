#ifndef alex_curve_DEFINED
#define alex_curve_DEFINED

static inline GPoint evalQuadPoint(GPoint a, GPoint b, GPoint c, float t) {
	return  a*((1-t)*(1-t)) + 2*b*t*(1-t) + c*(t*t);
}

static inline GPoint evalCubicPoint(GPoint a, GPoint b, GPoint c, GPoint d, float t) {
	return a*((1-t)*(1-t)*(1-t)) + 3*b*t*((1-t)*(1-t)) + 3*c*(1-t)*(t*t) + d*(t*t*t);
}

static inline GColor bilinearColor(float u, float v, GColor a, GColor b, GColor c, GColor d) {
	return (1-u)*(1-v)*a + u*(1-v)*b + (1-u)*v*d + u*v*c;
}

static inline GPoint bilinearPoint(float u, float v, GPoint a, GPoint b, GPoint c, GPoint d) {
	return (1-u)*(1-v)*a + u*(1-v)*b + (1-u)*v*d + u*v*c;
}

#endif