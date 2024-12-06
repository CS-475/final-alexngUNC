#include "include/GMatrix.h"

GMatrix::GMatrix() {
	fMat[0] = 1;	fMat[2] = 0;	fMat[4] = 0;
	fMat[1] = 0;	fMat[3] = 1;	fMat[5] = 0;
}

GMatrix GMatrix::Translate(float tx, float ty) {
	GMatrix m;
	m[0] = 1;	m[2] = 0;	m[4] = tx;
	m[1] = 0;	m[3] = 1;	m[5] = ty;
	return m;
}

GMatrix GMatrix::Scale(float sx, float sy) {
	GMatrix m;
	m[0] = sx;	m[2] = 0;	m[4] = 0;
	m[1] = 0;	m[3] = sy;	m[5] = 0;
	return m;
}

GMatrix GMatrix::Rotate(float radians) {
	GMatrix m;
	m[0] = std::cos(radians);	m[2] = -std::sin(radians);	m[4] = 0;
	m[1] = std::sin(radians);	m[3] = std::cos(radians);	m[5] = 0;
	return m;
}

GMatrix GMatrix::Concat(const GMatrix& a, const GMatrix& b) {
	GMatrix m;
	m[0] = a[0] * b[0] + a[2] * b[1];
	m[1] = a[1] * b[0] + a[3] * b[1];
	m[2] = a[0] * b[2] + a[2] * b[3];
	m[3] = a[1] * b[2] + a[3] * b[3];
	m[4] = a[0] * b[4] + a[2] * b[5] + a[4];
	m[5] = a[1] * b[4] + a[3] * b[5] + a[5];
	return m;
}

nonstd::optional<GMatrix> GMatrix::invert() const {
	float det = fMat[0] * fMat[3] - fMat[2] * fMat[1];
	if (det == 0.0f)
		return nonstd::nullopt;

	GMatrix m;
	float invDet = 1.0f / det;
	float a = fMat[0], b = fMat[1], c = fMat[2], d = fMat[3], e = fMat[4], f = fMat[5];
	m[0] = d * invDet;
	m[1] = -b * invDet;
	m[2] = -c * invDet;
	m[3] = a * invDet;
	m[4] = (c * f - d * e) * invDet;
	m[5] = (b * e - a * f) * invDet;
	return m;
}

void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const {
	for (int i=0; i<count; i++) {
		float sx = src[i].x;
		dst[i].x = fMat[0] * sx + fMat[2] * src[i].y + fMat[4];
		dst[i].y = fMat[1] * sx + fMat[3] * src[i].y + fMat[5];
	}
}