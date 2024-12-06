#include "include/GPath.h"
#include "include/GPathBuilder.h"
#include "alex_curve.h"

void GPathBuilder::addRect(const GRect& rect, GPathDirection dir) {
	GPoint start = {rect.left, rect.top};
	moveTo(start);
	GPoint p1, p2, p3;
	if (dir == GPathDirection::kCW) {
		p1 = {rect.right, rect.top};
		p2 = {rect.right, rect.bottom};
		p3 = {rect.left, rect.bottom};
	} else {
		p1 = {rect.left, rect.bottom};
		p2 = {rect.right, rect.bottom};
		p3 = {rect.right, rect.top};
	}
	lineTo(p1);
	lineTo(p2);
	lineTo(p3);
}

void GPathBuilder::addPolygon(const GPoint pts[], int count) {
	moveTo(pts[0]);
	for (int i=1; i<count; i++)
		lineTo(pts[i]);
}

// GRect GPath::bounds() const {
// 	if (countPoints() == 0) return {};
// 	GPoint p = fPts[0];
// 	float left = p.x;
// 	float right = p.x;
// 	float top = p.y;
// 	float bottom = p.y;
// 	for (int i=1; i<fPts.size(); i++) {
// 		p = fPts[i];
// 		if (p.x < left)
// 			left = p.x;
// 		else if (p.x > right)
// 			right = p.x;
// 		if (p.y < top)
// 			top = p.y;
// 		else if (p.y > bottom)
// 			bottom = p.y;
// 	}
// 	return GRect::LTRB(left, top, right, bottom);
// }

static inline float evalQuadFloat(float a, float b, float c, float t) {
	//todo: simplify
	return  a*((1-t)*(1-t)) + 2*b*t*(1-t) + c*(t*t);
}

static inline float evalCubicFloat(float a, float b, float c, float d, float t) {
	//todo simplify
	return a*((1-t)*(1-t)*(1-t)) + 3*b*t*((1-t)*(1-t)) + 3*c*(1-t)*(t*t) + d*(t*t*t);
}

struct CubicExtrema {
	float tArr[2];
	int count = 0;
};

GRect GPath::bounds() const {
	if (countPoints() == 0) return {};
	float gLeft = fPts[0].x;
	float gRight = gLeft;
	float gTop = fPts[0].y;
	float gBot = gTop;
	GPoint pts[GPath::kMaxNextPoints];
    GPath::Iter iter(*this);
	GPoint a, b, c, d;
	float t, p;
	CubicExtrema roots;
	auto updateBounds = [&](GPoint p) {
		if (p.x < gLeft)
			gLeft = p.x;
		else if (p.x > gRight)
			gRight = p.x;
		if (p.y < gTop)
			gTop = p.y;
		else if (p.y > gBot)
			gBot = p.y;
	};

	auto getQuadExtremum = [](float a, float b, float c) {
		float denom = a - 2*b + c;
		if (abs(denom) < 0.0001f) return 0.0f;
		return (a - b) / denom;
	};

	auto solveQuadForm = [](float a, float b, float c, float d) {
		float quadA = -a + 3*b - 3*c + d;
		float quadB = 2*a - 4*b + 2*c;
		float quadC = b - a;
		CubicExtrema result;
		if (abs(quadA) < 0.0001f) {
			if (quadB == 0.0f) return result;
			result.tArr[0] = -quadC / quadB;
			result.count = 1;
		} else {
			float insideSqrt = quadB*quadB - 4*quadA*quadC;
			if (insideSqrt < 0) return result;
			result.tArr[0] = (-quadB + sqrtf(insideSqrt)) / (2*quadA);
			result.tArr[1] = (-quadB - sqrtf(insideSqrt)) / (2*quadA);
			result.count = 2;
		}
		return result;
	};
	
    while (auto v = iter.next(pts)) {
    	switch (v.value()) {
			case GPathVerb::kMove:
				break;
			case GPathVerb::kLine:
				updateBounds(pts[0]);
				updateBounds(pts[1]);
				break;
			case GPathVerb::kQuad:
				a = pts[0];
				b = pts[1];
				c = pts[2];
				// first point
				updateBounds(a);
				// end point
				updateBounds(c);

				// extrema
				t = getQuadExtremum(a.x, b.x, c.x);
				if (t > 0 && t < 1) {
					p = evalQuadFloat(a.x, b.x, c.x, t);
					updateBounds({p, gTop});
				}
				t = getQuadExtremum(a.y, b.y, c.y);
				if (t > 0 && t < 1) {
					p = evalQuadFloat(a.y, b.y, c.y, t);
					updateBounds({gLeft, p});
				}
				break;
			case GPathVerb::kCubic:
				a = pts[0];
				b = pts[1];
				c = pts[2];
				d = pts[3];
				// first point
				updateBounds(a);
				// end point
				updateBounds(d);

				// extrema
				roots = solveQuadForm(a.x, b.x, c.x, d.x);
				for (int i=0; i<roots.count; i++) {
					if (roots.tArr[i] > 0 && roots.tArr[i] < 1) {
						p = evalCubicFloat(a.x, b.x, c.x, d.x, roots.tArr[i]);
						updateBounds({p, gTop});
					}
				}
				roots = solveQuadForm(a.y, b.y, c.y, d.y);
				for (int i=0; i<roots.count; i++) {
					if (roots.tArr[i] > 0 && roots.tArr[i] < 1) {
						p = evalCubicFloat(a.y, b.y, c.y, d.y, roots.tArr[i]);
						updateBounds({gLeft, p});
					}
				}
				break;
			default:
				break;
		}
	}
	return GRect::LTRB(gLeft, gTop, gRight, gBot);
}

void GPathBuilder::addCircle(GPoint center, float radius, GPathDirection dir) {
	//TODO: make math efficient (shift one instead of divide)
	//TODO: only use pts array
	//TODO: use float quadTo
	float tanPi8 = std::tan(gFloatPI/8);
	float negtanPi8 = -1*tanPi8;
	float sqrt2Over2 = sqrtf(2.0f) / 2.0f;
	float negSqrt2Over2 = -1*sqrt2Over2;
	GPoint a = {1.0f, 0.0f};
	GPoint b = {1.0f, negtanPi8};
	GPoint c = {sqrt2Over2, negSqrt2Over2};
	GPoint d = {tanPi8, -1.0f};
	GPoint e = {0.0f, -1.0f};
	GPoint f = {negtanPi8, -1.0f};
	GPoint g = {negSqrt2Over2, negSqrt2Over2};
	GPoint h = {-1.0f, negtanPi8};
	GPoint i = {-1.0f, 0.0f};
	GPoint j = {-1.0f, tanPi8};
	GPoint k = {negSqrt2Over2, sqrt2Over2};
	GPoint l = {negtanPi8, 1.0f};
	GPoint m = {0.0f, 1.0f};
	GPoint n = {tanPi8, 1.0f};
	GPoint o = {sqrt2Over2, sqrt2Over2};
	GPoint p = {1.0f, tanPi8};
	GPoint pts[16] = {a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p};
	GMatrix matrix = GMatrix(radius, 0.0f, center.x, 0.0f, radius, center.y);
	matrix.mapPoints(pts, 16);
	moveTo(pts[0]);
	if (dir == GPathDirection::kCW) {
		quadTo(pts[15], pts[14]);
		quadTo(pts[13], pts[12]);
		quadTo(pts[11], pts[10]);
		quadTo(pts[9], pts[8]);
		quadTo(pts[7], pts[6]);
		quadTo(pts[5], pts[4]);
		quadTo(pts[3], pts[2]);
		quadTo(pts[1], pts[0]);
	} else {
		quadTo(pts[1], pts[2]);
		quadTo(pts[3], pts[4]);
		quadTo(pts[5], pts[6]);
		quadTo(pts[7], pts[8]);
		quadTo(pts[9], pts[10]);
		quadTo(pts[11], pts[12]);
		quadTo(pts[13], pts[14]);
		quadTo(pts[15], pts[0]);
	}
}

void GPath::ChopQuadAt(const GPoint src[3], GPoint dst[5], float t) {
	GPoint a = src[0];
	GPoint b = src[1];
	GPoint c = src[2];
	GPoint bLeft = (1-t)*a + t*b;
	GPoint bRight = (1-t)*b + t*c;
	GPoint p = (1-t)*bLeft + t*bRight; 
	dst[0] = a;
	dst[1] = bLeft;
	dst[2] = p;
	dst[3] = bRight;
	dst[4] = c;
}

void GPath::ChopCubicAt(const GPoint src[4], GPoint dst[7], float t) {
	GPoint a = src[0];
	GPoint b = src[1];
	GPoint c = src[2];
	GPoint d = src[3];
	GPoint p0 = (1-t)*a + t*b;
	GPoint p1 = (1-t)*b + t*c;
	GPoint p2 = (1-t)*c + t*d;
	GPoint q0 = (1-t)*p0 + t*p1;
	GPoint q1 = (1-t)*p1 + t*p2;
	GPoint p = (1-t)*q0 + t*q1;
	dst[0] = a;
	dst[1] = p0;
	dst[2] = q0;
	dst[3] = p;
	dst[4] = q1;
	dst[5] = p2;
	dst[6] = d;
}