#ifndef alex_matrix_helpers_DEFINED
#define alex_matrix_helpers_DEFINED

static inline bool isScaleMatrix(GMatrix m) {
	return m[0] == m[3] && 
		   m[1] == 0 && 
		   m[2] == 0;
}

static inline bool isIdentity(GMatrix m) {
	return m[0] == 1 && m[2] == 0 &&
		   m[1] == 0 &&	m[3] == 1 &&
		   m[4] == 0 && m[5] == 0;
}

static inline GMatrix computeBases(GPoint p0, GPoint p1, GPoint p2) {
	float a = p1.x - p0.x;
	float b = p1.y - p0.y;
	float c = p2.x - p0.x;
	float d = p2.y - p0.y;
	return GMatrix(a, c, p0.x, b, d, p0.y);
}

#endif