/*
 *  Copyright 2024 <Alex Georgiev>
 */
#include "alex_canvas.h"
#include "alex_types.h"
#include "alex_utils.h"
#include "alex_blend.h"
#include "alex_shader.h"
#include "alex_matrix_helpers.h"
#include "alex_tree_shader.h"
#include "alex_curve.h"
#include <list>
#include "alex_tri_color_shader.h"
#include "alex_proxy_shader.h"
#include "alex_double_shader.h"

// Move loop termination into local variable
inline void MyCanvas::clear(const GColor& color) {
	// scale color
	unsigned alpha = GRoundToInt(color.a * 255.0f);
	int width = fDevice.width();
	int height = fDevice.height();
	GPixel *row_addr = nullptr;
	if (alpha == 0) {
		for (int y=0; y<height; y++) {
			row_addr = fDevice.getAddr(0, y);
			clearRow(row_addr, width);
		}
	} else {
		unsigned red = GRoundToInt(color.a * color.r * 255.0f);
		unsigned green = GRoundToInt(color.a * color.g * 255.0f);
		unsigned blue = GRoundToInt(color.a * color.b * 255.0f);
		GPixel pixel_color = GPixel_PackARGB(alpha, red, green, blue);
		for (int y=0; y<height; y++) {
			row_addr = fDevice.getAddr(0, y);
			storeRow(row_addr, width, pixel_color);
		}
	}
}

void MyCanvas::drawRect(const GRect& rect, const GPaint& paint) {
	if (!isIdentity(ctm)) {
		const GPoint points[] = { {rect.left, rect.top}, {rect.right, rect.top}, {rect.right, rect.bottom}, {rect.left, rect.bottom} };
		drawConvexPolygon(points, 4, paint);
		return;
	}

	GBlendMode blendMode = paint.getBlendMode();
	if (blendMode == GBlendMode::kDst)
		return;

	// src color
	GPixel src;

	// check if Paint is using color or ptr to shader for src
	bool usingShader = false;
	GShader *sh = paint.peekShader();
	if (sh != nullptr) {
		if (!(sh->setContext(ctm)))
			return;
		usingShader = true;
	} else {
		src = makePixelFromPaint(paint);
		// optimize blend mode
		blendMode = optimizeBlendMode(blendMode, src);
		if (blendMode == GBlendMode::kDst)
			return;
	}
	int iWidth = fDevice.width();
	float width = (float) iWidth;
	float height = (float) fDevice.height();
	int top = GRoundToInt(std::min(std::max(rect.top, 0.0f), height));
	int bottom = GRoundToInt(std::min(std::max(rect.bottom, 0.0f), height));
	int left = GRoundToInt(std::min(std::max(rect.left, 0.0f), width));
	int right = GRoundToInt(std::min(std::max(rect.right, 0.0f), width));
	if (left >= right || top >= bottom)
		return;
	int range = right - left;
	GPixel *row_addr = nullptr;
	if (usingShader) {
		if (sh->isOpaque())
			blendMode = optimizeOpaqueBlendMode(blendMode);
		GPixel srcRow[iWidth];
		switch (blendMode) {
			case GBlendMode::kClear:
				for (int y=top; y<bottom; y++) {
					row_addr = fDevice.getAddr(left, y);
					clearRow(row_addr, range);
				}
				break;
			case GBlendMode::kSrc:
				for (int y=top; y<bottom; y++) {
					row_addr = fDevice.getAddr(left, y);
					sh->shadeRow(left, y, range, srcRow);
					storeRowSR(row_addr, range, srcRow);	
				}
				break;
			case GBlendMode::kSrcOver:
				for (int y=top; y<bottom; y++) {
					row_addr = fDevice.getAddr(left, y);
					sh->shadeRow(left, y, range, srcRow);
					blitSrcOverSR(row_addr, range, srcRow);
				}
				break;
			case GBlendMode::kDstOver:
				for (int y=top; y<bottom; y++) {
					row_addr = fDevice.getAddr(left, y);
					sh->shadeRow(left, y, range, srcRow);
					blitDstOverSR(row_addr, range, srcRow);
				}
				break;	
			case GBlendMode::kSrcIn:
				for (int y=top; y<bottom; y++) {
					row_addr = fDevice.getAddr(left, y);
					sh->shadeRow(left, y, range, srcRow);
					blitSrcInSR(row_addr, range, srcRow);
				}
				break;
			case GBlendMode::kDstIn:
				for (int y=top; y<bottom; y++) {
					row_addr = fDevice.getAddr(left, y);
					sh->shadeRow(left, y, range, srcRow);
					blitDstInSR(row_addr, range, srcRow);
				}
				break;
			case GBlendMode::kSrcOut:
				for (int y=top; y<bottom; y++) {
					row_addr = fDevice.getAddr(left, y);
					sh->shadeRow(left, y, range, srcRow);
					blitSrcOutSR(row_addr, range, srcRow);
				}
				break;
			case GBlendMode::kDstOut:
				for (int y=top; y<bottom; y++) {
					row_addr = fDevice.getAddr(left, y);
					sh->shadeRow(left, y, range, srcRow);
					blitDstOutSR(row_addr, range, srcRow);
				}
				break;
			case GBlendMode::kSrcATop:
				for (int y=top; y<bottom; y++) {
					row_addr = fDevice.getAddr(left, y);
					sh->shadeRow(left, y, range, srcRow);
					blitSrcATopSR(row_addr, range, srcRow);
				}
				break;
			case GBlendMode::kDstATop:
				for (int y=top; y<bottom; y++) {
					row_addr = fDevice.getAddr(left, y);
					sh->shadeRow(left, y, range, srcRow);
					blitDstATopSR(row_addr, range, srcRow);
				}
				break;
			case GBlendMode::kXor:
				for (int y=top; y<bottom; y++) {
					row_addr = fDevice.getAddr(left, y);
					sh->shadeRow(left, y, range, srcRow);
					blitXorBlendSR(row_addr, range, srcRow);
				}
				break;
			default:
				break;
		}
	} else {
		switch (blendMode) {
			case GBlendMode::kClear:
				for (int y=top; y<bottom; y++) {
					row_addr = fDevice.getAddr(left, y);
					clearRow(row_addr, range);
				}
				break;
			case GBlendMode::kSrc:
				for (int y=top; y<bottom; y++) {
					row_addr = fDevice.getAddr(left, y);
					storeRow(row_addr, range, src);
				}
				break;
			case GBlendMode::kSrcOver:
				for (int y=top; y<bottom; y++) {
					row_addr = fDevice.getAddr(left, y);
					blitSrcOver(row_addr, range, src);
				}
				break;
			case GBlendMode::kDstOver:
				for (int y=top; y<bottom; y++) {
					row_addr = fDevice.getAddr(left, y);
					blitDstOver(row_addr, range, src);
				}
				break;	
			case GBlendMode::kSrcIn:
				for (int y=top; y<bottom; y++) {
					row_addr = fDevice.getAddr(left, y);
					blitSrcIn(row_addr, range, src);
				}
				break;
			case GBlendMode::kDstIn:
				for (int y=top; y<bottom; y++) {
					row_addr = fDevice.getAddr(left, y);
					blitDstIn(row_addr, range, src);
				}
				break;
			case GBlendMode::kSrcOut:
				for (int y=top; y<bottom; y++) {
					row_addr = fDevice.getAddr(left, y);
					blitSrcOut(row_addr, range, src);
				}
				break;
			case GBlendMode::kDstOut:
				for (int y=top; y<bottom; y++) {
					row_addr = fDevice.getAddr(left, y);
					blitDstOut(row_addr, range, src);
				}
				break;
			case GBlendMode::kSrcATop:
				for (int y=top; y<bottom; y++) {
					row_addr = fDevice.getAddr(left, y);
					blitSrcATop(row_addr, range, src);
				}
				break;
			case GBlendMode::kDstATop:
				for (int y=top; y<bottom; y++) {
					row_addr = fDevice.getAddr(left, y);
					blitDstATop(row_addr, range, src);
				}
				break;
			case GBlendMode::kXor:
				for (int y=top; y<bottom; y++) {
					row_addr = fDevice.getAddr(left, y);
					blitXorBlend(row_addr, range, src);
				}
				break;
			default:
				break;
		}
	}
}

/*
void MyCanvas::drawRect(const GRect& rect, const GPaint& paint) {
	const GPoint points[] = { {rect.left, rect.top}, {rect.right, rect.top}, {rect.right, rect.bottom}, {rect.left, rect.bottom} };
	drawConvexPolygon(points, 4, paint);
}
*/


inline void makeEdge(Edge& e, GPoint p0, GPoint p1, float m, float b) {
	e.top = GRoundToInt(std::min(p0.y, p1.y));
	e.bottom = GRoundToInt(std::max(p0.y, p1.y));
	e.m = m;
	e.b = b;
}

inline float clipAboveBelow(GPoint& p, float mx, int clipTop) {
	return p.x + (clipTop - p.y) * mx;
}

inline float clipSide(GPoint& p, float my, int side) {
	return p.y + (side - p.x) * my;
}

inline void swapPoints(GPoint& p0, GPoint& p1) {
	GPoint temp = p1;
	p1 = p0;
	p0 = temp;
}

inline void lineToEdges(std::vector<Edge>& edges, GPoint p0, GPoint p1, int height, int width) {
	// swap so that p1 is below
	if (p0.y > p1.y)
		swapPoints(p0, p1);

	// skip if both above
	if (p1.y < 0)
		return;
	// skip if both below
	if (p0.y > height)
		return;

	// check for horizontal lines
	int bottom = GRoundToInt(p1.y);
	int top = GRoundToInt(p0.y);
	if (top >= bottom)
		return;
	Edge e;
	GPoint proj;

	float mx = (p1.x - p0.x) / (p1.y - p0.y);
	float my = (p1.y - p0.y) / (p1.x - p0.x);
	float b = p0.x - p0.y * mx;

	// straddle top
	if (p0.y < 0) {
		p0.x = clipAboveBelow(p0, mx, 0);
		p0.y = 0;
	}

	// straddle bottom
	if (p1.y > height) {
		p1.x = clipAboveBelow(p1, mx, height);
		p1.y = height;
	}

	// put p0 on left
	if (p0.x > p1.x)
		swapPoints(p0, p1);

	// chop/vertical segment for left and right boundaries
	// if both on left
	if (p1.x < 0) {
		p0.x = 0;
		p1.x = 0;
		makeEdge(e, p0, p1, 0, 0);
		edges.push_back(e);
		return;
	}
	
	// if both on right
	if (p0.x > width) {
		p0.x = width;
		p1.x = width;
		makeEdge(e, p0, p1, 0, width);
		edges.push_back(e);
		return;
	}

	// straddle left
	if (p0.x < 0) {
		proj.x = 0;
		proj.y = p0.y;
		p0.y = clipSide(p0, my, 0);
		p0.x = 0;
		makeEdge(e, proj, p0, 0, 0);
		edges.push_back(e);
	}

	// straddle right
	if (p1.x > width) {
		proj.x = width;
		proj.y = p1.y;
		p1.y = clipSide(p1, my, width);
		p1.x = width;
		makeEdge(e, proj, p1, 0, width);
		if (e.top < e.bottom)
			edges.push_back(e);
	}
	
	makeEdge(e, p0, p1, mx, b);
	edges.push_back(e);
}

void lineToClippedWindingEdges(std::vector<Edge>& edges, GPoint p0, GPoint p1, int height, int width) {
	// calculate winding value
	int winding = -1;

	// swap so that p1 is below
	if (p0.y > p1.y) {
		winding = 1;
		swapPoints(p0, p1);
	}

	// skip if both above
	if (p1.y < 0)
		return;
	// skip if both below
	if (p0.y > height)
		return;

	// check for horizontal lines
	int bottom = GRoundToInt(p1.y);
	int top = GRoundToInt(p0.y);
	if (top >= bottom)
		return;

	Edge e;
	GPoint proj;
	float mx = (p1.x - p0.x) / (p1.y - p0.y);
	float my = (p1.y - p0.y) / (p1.x - p0.x);
	float b = p0.x - p0.y * mx;

	// straddle top
	if (p0.y < 0) {
		p0.x = clipAboveBelow(p0, mx, 0);
		p0.y = 0;
	}

	// straddle bottom
	if (p1.y > height) {
		p1.x = clipAboveBelow(p1, mx, height);
		p1.y = height;
	}

	// put p0 on left
	if (p0.x > p1.x) {
		swapPoints(p0, p1);
	}

	// chop/vertical segment for left and right boundaries
	// if both on left
	if (p1.x < 0) {
		p0.x = 0;
		p1.x = 0;
		makeEdge(e, p0, p1, 0, 0);
		e.w = winding;
		if (e.top < e.bottom)
			edges.push_back(e);
		return;
	}
	
	// if both on right
	if (p0.x > width) {
		p0.x = width;
		p1.x = width;
		makeEdge(e, p0, p1, 0, width);
		e.w = winding;
		if (e.top < e.bottom)
			edges.push_back(e);
		return;
	}

	// straddle left
	if (p0.x < 0) {
		proj.x = 0;
		proj.y = p0.y;
		p0.y = clipSide(p0, my, 0);
		p0.x = 0;
		makeEdge(e, proj, p0, 0, 0);
		e.w = winding;
		if (e.top < e.bottom)
			edges.push_back(e);
	}

	// straddle right
	if (p1.x > width) {
		proj.x = width;
		proj.y = p1.y;
		p1.y = clipSide(p1, my, width);
		p1.x = width;
		makeEdge(e, proj, p1, 0, width);
		e.w = winding;
		if (e.top < e.bottom)
			edges.push_back(e);
	}
	
	makeEdge(e, p0, p1, mx, b);
	e.w = winding;
	if (e.top < e.bottom)
		edges.push_back(e);
}

inline void pointsToEdges(std::vector<Edge>& edges, const GPoint points[], unsigned n, int height, int width) {
	for (int i=0; i<n-1; i++) {
		lineToEdges(edges, points[i], points[i+1], height, width);
	}
	// closure
	lineToEdges(edges, points[n-1], points[0], height, width);
}

inline void sortEdgesTop(std::vector<Edge>& edges) {
	std::sort(edges.begin(), edges.end(), [](const Edge& a, const Edge& b) {
        return a.top < b.top;
    });
}

inline void checkExpiration(Edge& firstEdge, Edge& secondEdge, int& nextIdx, std::vector<Edge>& edges, int y) {
	// check if first edge expired
	if (firstEdge.bottom == y) {
		firstEdge = edges[nextIdx];
		nextIdx += 1;
	}
	// check if second edge expired
	if (secondEdge.bottom == y) {
		secondEdge = edges[nextIdx];
		nextIdx += 1;
	}
}

inline void shootRay(int& left, int& right, Edge firstEdge, Edge secondEdge, float ray) {
	// find left and right bounds for row
	float x = firstEdge.m * ray + firstEdge.b;
	int firstX = GRoundToInt(x);
	x = secondEdge.m * ray + secondEdge.b;
	int secondX = GRoundToInt(x);
	// blit the row
	left = std::min(firstX, secondX);
	right = std::max(firstX, secondX);
}

void MyCanvas::drawConvexPolygon(const GPoint points[], int count, const GPaint& paint) {
	if (count < 3)
		return;
	GBlendMode blendMode = paint.getBlendMode();
	if (blendMode == GBlendMode::kDst)
		return;

	// src color
	GPixel src;
	
	// check if Paint is using color or ptr to shader for src
	bool usingShader = false;
	GShader *sh = paint.peekShader();
	if (sh != nullptr) {
		if (!(sh->setContext(ctm)))
			return;
		usingShader = true;
	} else {
		src = makePixelFromPaint(paint);
		// optimize blend mode
		blendMode = optimizeBlendMode(blendMode, src);
		if (blendMode == GBlendMode::kDst)
			return;
	}

	// Transform points
	GPoint mapped_points[count];
	ctm.mapPoints(mapped_points, points, count);

	// Device dimensions
	int height = fDevice.height();
	int width = fDevice.width();

	// make edges
	std::vector<Edge> edges;
	edges.reserve(2 * count);
	pointsToEdges(edges, mapped_points, count, height, width);
	int numEdges = edges.size();
	if (numEdges < 2)
		return;

	// sort edges by top
	sortEdgesTop(edges);

	// loop and intersect per y + 0.5
	int maxEdgeIdx = numEdges - 1;
	int top = edges[0].top;
	int bottom = edges[maxEdgeIdx].bottom;

	// edges blitting between
	Edge firstEdge = edges[0];
	Edge secondEdge = edges[1];

	// idx for assigning the next edge once edges expire
	int nextIdx = 2;

	// left and right blit row bounds
	int left = 0;
	int right = 0;
	GPixel *row_addr = nullptr;
	float ray = 0.0f;
	int range = 0;
	if (usingShader) {
		if (sh->isOpaque())
			blendMode = optimizeOpaqueBlendMode(blendMode);
		GPixel srcRow[width];
		switch (blendMode) {
			case GBlendMode::kClear:
				for (int y=top; y<bottom; y++) {
					checkExpiration(firstEdge, secondEdge, nextIdx, edges, y);
					ray = y + 0.5f;
					shootRay(left, right, firstEdge, secondEdge, ray);
					range = right - left;
					if (range > 0) {
						row_addr = fDevice.getAddr(left, y);
						clearRow(row_addr, range);
					}
				}
				break;
			case GBlendMode::kSrc:
				for (int y=top; y<bottom; y++) {
					checkExpiration(firstEdge, secondEdge, nextIdx, edges, y);
					ray = y + 0.5f;
					shootRay(left, right, firstEdge, secondEdge, ray);
					range = right - left;
					if (range > 0) {
						row_addr = fDevice.getAddr(left, y);
						sh->shadeRow(left, y, range, srcRow);
						storeRowSR(row_addr, range, srcRow);
					}	
				}
				break;
			case GBlendMode::kSrcOver:
				for (int y=top; y<bottom; y++) {
					checkExpiration(firstEdge, secondEdge, nextIdx, edges, y);
					ray = y + 0.5f;
					shootRay(left, right, firstEdge, secondEdge, ray);
					range = right - left;
					if (range > 0) {
						row_addr = fDevice.getAddr(left, y);
						sh->shadeRow(left, y, range, srcRow);
						blitSrcOverSR(row_addr, range, srcRow);
					}
				}
				break;
			case GBlendMode::kDstOver:
				for (int y=top; y<bottom; y++) {
					checkExpiration(firstEdge, secondEdge, nextIdx, edges, y);
					ray = y + 0.5f;
					shootRay(left, right, firstEdge, secondEdge, ray);
					range = right - left;
					if (range > 0) {
						row_addr = fDevice.getAddr(left, y);
						sh->shadeRow(left, y, range, srcRow);
						blitDstOverSR(row_addr, range, srcRow);
					}
				}
				break;
			case GBlendMode::kSrcIn:
				for (int y=top; y<bottom; y++) {
					checkExpiration(firstEdge, secondEdge, nextIdx, edges, y);
					ray = y + 0.5f;
					shootRay(left, right, firstEdge, secondEdge, ray);
					range = right - left;
					if (range > 0) {
						row_addr = fDevice.getAddr(left, y);
						sh->shadeRow(left, y, range, srcRow);
						blitSrcInSR(row_addr, range, srcRow);
					}
				}
				break;
			case GBlendMode::kDstIn:
				for (int y=top; y<bottom; y++) {
					checkExpiration(firstEdge, secondEdge, nextIdx, edges, y);
					ray = y + 0.5f;
					shootRay(left, right, firstEdge, secondEdge, ray);
					range = right - left;
					if (range > 0) {
						row_addr = fDevice.getAddr(left, y);
						sh->shadeRow(left, y, range, srcRow);
						blitDstInSR(row_addr, range, srcRow);
					}
				}
				break;
			case GBlendMode::kSrcOut:
				for (int y=top; y<bottom; y++) {
					checkExpiration(firstEdge, secondEdge, nextIdx, edges, y);
					ray = y + 0.5f;
					shootRay(left, right, firstEdge, secondEdge, ray);
					range = right - left;
					if (range > 0) {
						row_addr = fDevice.getAddr(left, y);
						sh->shadeRow(left, y, range, srcRow);
						blitSrcOutSR(row_addr, range, srcRow);
					}
				}
				break;
			case GBlendMode::kDstOut:
				for (int y=top; y<bottom; y++) {
					checkExpiration(firstEdge, secondEdge, nextIdx, edges, y);
					ray = y + 0.5f;
					shootRay(left, right, firstEdge, secondEdge, ray);
					range = right - left;
					if (range > 0) {
						row_addr = fDevice.getAddr(left, y);
						sh->shadeRow(left, y, range, srcRow);
						blitDstOutSR(row_addr, range, srcRow);
					}
				}
				break;
			case GBlendMode::kSrcATop:
				for (int y=top; y<bottom; y++) {
					checkExpiration(firstEdge, secondEdge, nextIdx, edges, y);
					ray = y + 0.5f;
					shootRay(left, right, firstEdge, secondEdge, ray);
					range = right - left;
					if (range > 0) {
						row_addr = fDevice.getAddr(left, y);
						sh->shadeRow(left, y, range, srcRow);
						blitSrcATopSR(row_addr, range, srcRow);
					}
				}
				break;
			case GBlendMode::kDstATop:
				for (int y=top; y<bottom; y++) {
					checkExpiration(firstEdge, secondEdge, nextIdx, edges, y);
					ray = y + 0.5f;
					shootRay(left, right, firstEdge, secondEdge, ray);
					range = right - left;
					if (range > 0) {
						row_addr = fDevice.getAddr(left, y);
						sh->shadeRow(left, y, range, srcRow);
						blitDstATopSR(row_addr, range, srcRow);
					}
				}
				break;
			case GBlendMode::kXor:
				for (int y=top; y<bottom; y++) {
					checkExpiration(firstEdge, secondEdge, nextIdx, edges, y);
					ray = y + 0.5f;
					shootRay(left, right, firstEdge, secondEdge, ray);
					range = right - left;
					if (range > 0) {
						row_addr = fDevice.getAddr(left, y);
						sh->shadeRow(left, y, range, srcRow);
						blitXorBlendSR(row_addr, range, srcRow);
					}
				}
				break;
			default:
				break;
		}
	} else {
		switch (blendMode) {
			case GBlendMode::kClear:
				for (int y=top; y<bottom; y++) {
					checkExpiration(firstEdge, secondEdge, nextIdx, edges, y);
					ray = y + 0.5f;
					shootRay(left, right, firstEdge, secondEdge, ray);
					range = right - left;
					if (range > 0) {
						row_addr = fDevice.getAddr(left, y);
						clearRow(row_addr, range);
					}
				}
				break;
			case GBlendMode::kSrc:
				for (int y=top; y<bottom; y++) {
					checkExpiration(firstEdge, secondEdge, nextIdx, edges, y);
					ray = y + 0.5f;
					shootRay(left, right, firstEdge, secondEdge, ray);
					range = right - left;
					if (range > 0) {
						row_addr = fDevice.getAddr(left, y);
						storeRow(row_addr, range, src);
					}
				}
				break;
			case GBlendMode::kSrcOver:
				for (int y=top; y<bottom; y++) {
					checkExpiration(firstEdge, secondEdge, nextIdx, edges, y);
					ray = y + 0.5f;
					shootRay(left, right, firstEdge, secondEdge, ray);
					range = right - left;
					if (range > 0) {
						row_addr = fDevice.getAddr(left, y);
						blitSrcOver(row_addr, range, src);
					}
				}
				break;
			case GBlendMode::kDstOver:
				for (int y=top; y<bottom; y++) {
					checkExpiration(firstEdge, secondEdge, nextIdx, edges, y);
					ray = y + 0.5f;
					shootRay(left, right, firstEdge, secondEdge, ray);
					range = right - left;
					if (range > 0) {
						row_addr = fDevice.getAddr(left, y);
						blitDstOver(row_addr, range, src);
					}
				}
				break;	
			case GBlendMode::kSrcIn:
				for (int y=top; y<bottom; y++) {
					checkExpiration(firstEdge, secondEdge, nextIdx, edges, y);
					ray = y + 0.5f;
					shootRay(left, right, firstEdge, secondEdge, ray);
					range = right - left;
					if (range > 0) {
						row_addr = fDevice.getAddr(left, y);
						blitSrcIn(row_addr, range, src);
					}
				}
				break;
			case GBlendMode::kDstIn:
				for (int y=top; y<bottom; y++) {
					checkExpiration(firstEdge, secondEdge, nextIdx, edges, y);
					ray = y + 0.5f;
					shootRay(left, right, firstEdge, secondEdge, ray);
					range = right - left;
					if (range > 0) {
						row_addr = fDevice.getAddr(left, y);
						blitDstIn(row_addr, range, src);
					}
				}
				break;
			case GBlendMode::kSrcOut:
				for (int y=top; y<bottom; y++) {
					checkExpiration(firstEdge, secondEdge, nextIdx, edges, y);
					ray = y + 0.5f;
					shootRay(left, right, firstEdge, secondEdge, ray);
					range = right - left;
					if (range > 0) {
						row_addr = fDevice.getAddr(left, y);
						blitSrcOut(row_addr, range, src);
					}
				}
				break;
			case GBlendMode::kDstOut:
				for (int y=top; y<bottom; y++) {
					checkExpiration(firstEdge, secondEdge, nextIdx, edges, y);
					ray = y + 0.5f;
					shootRay(left, right, firstEdge, secondEdge, ray);
					range = right - left;
					if (range > 0) {
						row_addr = fDevice.getAddr(left, y);
						blitDstOut(row_addr, range, src);
					}
				}
				break;
			case GBlendMode::kSrcATop:
				for (int y=top; y<bottom; y++) {
					checkExpiration(firstEdge, secondEdge, nextIdx, edges, y);
					ray = y + 0.5f;
					shootRay(left, right, firstEdge, secondEdge, ray);
					range = right - left;
					if (range > 0) {
						row_addr = fDevice.getAddr(left, y);
						blitSrcATop(row_addr, range, src);
					}
				}
				break;
			case GBlendMode::kDstATop:
				for (int y=top; y<bottom; y++) {
					checkExpiration(firstEdge, secondEdge, nextIdx, edges, y);
					ray = y + 0.5f;
					shootRay(left, right, firstEdge, secondEdge, ray);
					range = right - left;
					if (range > 0) {
						row_addr = fDevice.getAddr(left, y);
						blitDstATop(row_addr, range, src);
					}
				}
				break;
			case GBlendMode::kXor:
				for (int y=top; y<bottom; y++) {
					checkExpiration(firstEdge, secondEdge, nextIdx, edges, y);
					ray = y + 0.5f;
					shootRay(left, right, firstEdge, secondEdge, ray);
					range = right - left;
					if (range > 0) {
						row_addr = fDevice.getAddr(left, y);
						blitXorBlend(row_addr, range, src);
					}
				}
				break;
			default:
				break;
		}
	}
}


inline void sortEdgesTopAndX(std::vector<Edge>& edges) {
	std::sort(edges.begin(), edges.end(), [](const Edge& a, const Edge& b) {
        if (a.top != b.top) {
			// Sort by top first
            return a.top < b.top; 
        }
		// If tops are equal, sort by x
        return a.x < b.x;
    });	
}

// inline void sortEdgesX(std::vector<Edge>& edges, size_t k) {
// 	assert(k <= edges.size());
// 	std::sort(edges.begin(), edges.begin() + k, [](const Edge& a, const Edge& b) {
//         return a.x < b.x;
//     });
// }

inline void sortEdgesX(std::list<Edge>& edges, size_t k) {
    assert(k <= edges.size()); // Ensure k is not greater than the list size
	// temp vector
	std::vector<Edge> temp;
    auto it = edges.begin();
    for (size_t i = 0; i < k; ++i) {
        temp.push_back(*it);
        ++it;
    }

    // sort the vector
    std::sort(temp.begin(), temp.end(), [](const Edge& a, const Edge& b) {
        return a.x < b.x;
    });

    // replace the first k elements in the list with the sorted elements
    it = edges.begin();
    for (const auto& edge : temp) {
        *it = edge;
        ++it;
    }
}

inline void makeEdgeFromArgs(Edge& e, int top, int bottom, float m, float b, int w) {
	e.top = top;
	e.bottom = bottom; 
	e.m = m;
	e.b = b;
	e.w = w;
}

// PA 4
void MyCanvas::drawPath(const GPath& path, const GPaint& paint) {
	size_t count = path.countPoints();
	if (count < 3) return;
	GBlendMode blendMode = paint.getBlendMode();
	if (blendMode == GBlendMode::kDst) return;

	// src color
	GPixel src;

	// Check if paint is using color or ptr to shader for src
	bool usingShader = false;
	GShader *sh = paint.peekShader();
	if (sh != nullptr) {
		if (!(sh->setContext(ctm))) return;
		usingShader = true;
	} else {
		src = makePixelFromPaint(paint);
		// optimize blend mode
		blendMode = optimizeBlendMode(blendMode, src);
		if (blendMode == GBlendMode::kDst) return;
	}
	// if rect is outside of bounds, return
	// if rect is all in bounds, don't clip

	// Device dimensions
	int height = fDevice.height();
	int width = fDevice.width();

	// Get edges from path
	std::shared_ptr<GPath> transformedPath = path.transform(ctm);
	GPath::Edger edger(*transformedPath);
	GPoint pts[GPath::kMaxNextPoints];
	// check if clipping is neccessary
	// GRect bounding_box = transformedPath->bounds();
	bool needClip = true;
	//TODO: fix bounds
	// if (bounding_box.right < 0 ||
	// 	bounding_box.bottom < 0 ||
	// 	bounding_box.left > width ||
	// 	bounding_box.top > height) {
	// 	return;
	// } else if (bounding_box.left >= 0 &&
    //             bounding_box.right <= width &&
    //             bounding_box.top >= 0 &&
    //             bounding_box.bottom <= height) {
	// 				needClip = false;
	// }
	std::vector<Edge> edges;
	Edge e;
	edges.reserve(2 * count);
	GPoint error, error2, p0, p1;
	float tolerance = 1.0f/4.0f;
	float mx, b, t, dt;
	int num_segs, w, edgeTop, edgeBottom;
	if (needClip) {
		while (auto v = edger.next(pts)) {
			switch (v.value()) {
				case GPathVerb::kLine:
					lineToClippedWindingEdges(edges, pts[0], pts[1], height, width);
					break;
				case GPathVerb::kQuad:
					p0 = pts[0];
					p1 = pts[0];
					error = (pts[0] - 2*pts[1] + pts[2])*(1.0f/4.0f);
					num_segs = (int)ceil(sqrt(error.length()/tolerance));
					t = 0.0f;
					dt = 1.0f / num_segs;
					for (int i=0; i<num_segs-1; i++) {
						//p0 = evalQuadPoint(pts[0], pts[1], pts[2], t);
						p1 = evalQuadPoint(pts[0], pts[1], pts[2], t + dt);
						lineToClippedWindingEdges(edges, p0, p1, height, width);
						p0 = p1;
						t += dt;
					}
					lineToClippedWindingEdges(edges, p1, pts[2], height, width);
					break;
				case GPathVerb::kCubic:
					error = pts[0] - 2*pts[1] + pts[2];
					error2 = pts[1] - 2*pts[2] + pts[3];
					error.x = std::max(abs(error.x), abs(error2.x));
					error.y = std::max(abs(error.y), abs(error2.y));
					num_segs = (int)ceil(sqrt((3*error.length())/(4.0f*tolerance)));
					t = 0.0f;
					dt = 1.0f / num_segs;
					p0 = pts[0];
					p1 = pts[0];
					for (int i=0; i<num_segs-1; i++) {
						//p0 = evalCubicPoint(pts[0], pts[1], pts[2], pts[3], t);
						p1 = evalCubicPoint(pts[0], pts[1], pts[2], pts[3], t + dt);
						lineToClippedWindingEdges(edges, p0, p1, height, width);
						p0 = p1;
						t += dt;
					}
					lineToClippedWindingEdges(edges, p1, pts[3], height, width);
					break;
				default:
					break;
			}
		}
	} else {
		while (auto v = edger.next(pts)) {
			switch (v.value()) {
				case GPathVerb::kLine:
					p0 = pts[0];
					p1 = pts[0];
					edgeTop = GRoundToInt(std::min(p0.y, p1.y));
					edgeBottom = GRoundToInt(std::max(p0.y, p1.y));
					if (edgeTop >= edgeBottom)
						continue;
					mx = (p1.x - p0.x) / (p1.y - p0.y);
					b = p0.x - p0.y * mx;
					w = -1;
					if (p0.y > p1.y)
						w = 1;
					makeEdgeFromArgs(e, edgeTop, edgeBottom, mx, b, w);
					edges.push_back(e);
					break;
				case GPathVerb::kQuad:
					error = (pts[0] - 2*pts[1] + pts[2])*(1.0f/4.0f);
					num_segs = (int)ceil(sqrt(error.length()/tolerance));
					t = 0.0f;
					dt = 1.0f / num_segs;
					p0 = pts[0];
					p1 = pts[0];
					for (int i=0; i<num_segs-1; i++) {
						//p0 = evalQuadPoint(pts[0], pts[1], pts[2], t);
						p1 = evalQuadPoint(pts[0], pts[1], pts[2], t + dt);
						edgeTop = GRoundToInt(std::min(p0.y, p1.y));
						edgeBottom = GRoundToInt(std::max(p0.y, p1.y));
						if (edgeTop >= edgeBottom)
							continue;
						mx = (p1.x - p0.x) / (p1.y - p0.y);
						b = p0.x - p0.y * mx;
						w = -1;
						if (p0.y > p1.y)
							w = 1;
						makeEdgeFromArgs(e, edgeTop, edgeBottom, mx, b, w);
						edges.push_back(e);
						t += dt;
						p0 = p1;
					}
					p0 = p1;
					p1 = pts[2];
					edgeTop = GRoundToInt(std::min(p0.y, p1.y));
					edgeBottom = GRoundToInt(std::max(p0.y, p1.y));
					if (edgeTop >= edgeBottom)
						continue;
					mx = (p1.x - p0.x) / (p1.y - p0.y);
					b = p0.x - p0.y * mx;
					w = -1;
					if (p0.y > p1.y)
						w = 1;
					makeEdgeFromArgs(e, edgeTop, edgeBottom, mx, b, w);
					edges.push_back(e);
					break;
				case GPathVerb::kCubic:
					error = pts[0] - 2*pts[1] + pts[2];
					error2 = pts[1] - 2*pts[2] + pts[3];
					error.x = std::max(abs(error.x), abs(error2.x));
					error.y = std::max(abs(error.y), abs(error2.y));
					num_segs = (int)ceil(sqrt((3*error.length())/(4.0f*tolerance)));
					t = 0.0f;
					dt = 1.0f / num_segs;
					p0 = pts[0];
					p1 = pts[0];
					for (int i=0; i<num_segs-1; i++) {
						//p0 = evalCubicPoint(pts[0], pts[1], pts[2], pts[3], t);
						p1 = evalCubicPoint(pts[0], pts[1], pts[2], pts[3], t + dt);
						edgeTop = GRoundToInt(std::min(p0.y, p1.y));
						edgeBottom = GRoundToInt(std::max(p0.y, p1.y));
						if (edgeTop >= edgeBottom)
							continue;
						mx = (p1.x - p0.x) / (p1.y - p0.y);
						b = p0.x - p0.y * mx;
						w = -1;
						if (p0.y > p1.y)
							w = 1;
						makeEdgeFromArgs(e, edgeTop, edgeBottom, mx, b, w);
						edges.push_back(e);	
						p0 = p1;
						t += dt;
					}
					p0 = p1;
					p1 = pts[3];
					edgeTop = GRoundToInt(std::min(p0.y, p1.y));
					edgeBottom = GRoundToInt(std::max(p0.y, p1.y));
					if (edgeTop >= edgeBottom)
						continue;
					mx = (p1.x - p0.x) / (p1.y - p0.y);
					b = p0.x - p0.y * mx;
					w = -1;
					if (p0.y > p1.y)
						w = 1;
					makeEdgeFromArgs(e, edgeTop, edgeBottom, mx, b, w);
					edges.push_back(e);
					break;
				default:
					break;
			}
		}
	}

	size_t numEdges = edges.size();
	if (numEdges < 2) return;

	// find top and bottom and first ray intersect x
	int top = edges[0].top;
	int bottom = edges[0].bottom;
	for (int i=1; i<numEdges; i++) {
		if (edges[i].top < top)
			top = edges[i].top;
		if (edges[i].bottom > bottom)
			bottom = edges[i].bottom;
	}

	for (int i=0; i<numEdges; i++) {
		edges[i].x = edges[i].computeX(top);
	}

	// sort edges based on top and x
	sortEdgesTopAndX(edges);

	// linked list for deletion
	std::list<Edge> edgeList(edges.begin(), edges.end());
	auto it = edgeList.begin();
	// loop through all y's containing edges
	int blendModeInt = (int) blendMode;
	int range = 0;
	GPixel *row_addr = nullptr;
	
	if (usingShader) {
		if (sh->isOpaque())
			blendMode = optimizeOpaqueBlendMode(blendMode);
		GPixel srcRow[width];
		BlitRowSRProc blitRowSR = gblitRowSRProcs[blendModeInt];
		for (int y=top; y<bottom; y++) {
			size_t i = 0;
			int w = 0;
			int L = 0;
			// loop through actice edges for this y value
			while (i < numEdges && (*it).valid(y)) {
				int x = GRoundToInt((*it).computeX(y));
				if (w == 0)
					L = x;
				w += (*it).w;
				if (w == 0) {
					int R = x;
					range = R - L;
					if (range > 0) {
						row_addr = fDevice.getAddr(L, y);
						sh->shadeRow(L, y, range, srcRow);
						blitRowSR(row_addr, range, srcRow);
					}
				}

				if ((*it).valid(y+1)) {
					i += 1;
					++it;
				} else {
					// TODO: fix
					it = edgeList.erase(it);
					numEdges -= 1;
				}
			}
			assert(w == 0);
			// now i is the number of remaining valid edges

			// account for new edges that will be valid for next y
			while (i < numEdges && (*it).valid(y+1)) {
				i += 1;
				++it;
			}

			// now i also includes the number of edges that will be valid
			// i = std::min(i, edges.size());
			it = edgeList.begin();
			for (int j=0; j<i; j++) {
				(*it).x = (*it).computeX(y+1);
				++it;
			}
			// edges.assign(edgeList.begin(), edgeList.end());
			sortEdgesX(edgeList, i);
			// edgeList.assign(edges.begin(), edges.end());
			it = edgeList.begin();
		}
	} else {
		BlitRowProc blitRow = gblitRowProcs[blendModeInt];
		for (int y=top; y<bottom; y++) {
			size_t i = 0;
			int w = 0;
			int L = 0;
			// loop through active edges for this y value
			while (i < numEdges && (*it).valid(y)) {
				int x = GRoundToInt((*it).computeX(y));
				if (w == 0)
					L = x;
				w += (*it).w;
				if (w == 0) {
					int R = x;
					range = R - L;
					if (range > 0) {
						row_addr = fDevice.getAddr(L, y);
						blitRow(row_addr, range, src);
					}
				}

				if ((*it).valid(y+1)) {
					i += 1;
					++it;
				} else {
					it = edgeList.erase(it);
					numEdges -= 1;
				}
			}
			assert(w == 0);
			// now i is the number of remaining valid edges

			// account for new edges that will be valid for next y
			while (i < numEdges && (*it).valid(y+1)) {
				i += 1;
				++it;
			}

			// now i also includes the number of edges that will be valid
			// i = std::min(i, edges.size());
			it = edgeList.begin();
			for (int j=0; j<i; j++) {
				(*it).x = (*it).computeX(y+1);
				++it;
			}
			// edges.assign(edgeList.begin(), edgeList.end());
			sortEdgesX(edgeList, i);
			// edgeList.assign(edges.begin(), edges.end());
			it = edgeList.begin();
		}	
	}
}

std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& device) {
	//return std::make_unique<MyCanvas>(device);
    return std::unique_ptr<GCanvas>(new MyCanvas(device));
}

/* PA 2 drawSomething
std::string GDrawSomething(GCanvas* canvas, GISize dim) {
    // as fancy as you like
    // ...
    // canvas->clear(...);
    // canvas->fillRect(...);
	// canvas->clear(GColor::RGBA(1.0f, 1.0f, 1.0f, 1.0f));

	// make the ocean
    // GRect r = GRect::LTRB(0.0f, dim.height - 80.0f, dim.width, dim.height);
	// GColor color = GColor::RGBA(0.0f, 0.45f, 1.0f, 1.0f);
	// canvas->fillRect(r, color);

	// // make the beach
	// r = GRect::LTRB(0.0f, dim.height - 130.0f, dim.width, dim.height - 80.0f);
	// color = GColor::RGBA(0.7f, 0.7f, 0.5f, 1.0f);
	// canvas->fillRect(r, color);

	// // paint the sky
	// color = GColor::RGBA(0.0f, 0.7f, 1.0f, 1.0f);
	// r = GRect::LTRB(0.0f, 0.0f, dim.width, dim.height - 130.0f);
	// canvas->fillRect(r, color);

	// // add the sun
	// color = GColor::RGBA(1.0f, 1.0f, 0.0f, 1.0f);
	// r = GRect::LTRB(0.0f, 0.0f, 50.0f, 50.0f);
	// canvas->fillRect(r, color);

	// // add tree trunk
	// color = GColor::RGBA(0.2f, 0.2f, 0.2f, 1.0f);
	// r = GRect::LTRB(dim.width - 70.0f, 80.0f, dim.width - 50.0f, dim.height - 100.0f);
	// canvas->fillRect(r, color);

	// // add tree leaves
	// color = GColor::RGBA(0.0f, 0.5f, 0.0f, 1.0f);
	// GPaint paint = GPaint(color);
	// GPoint points[] = {{dim.width - 110.0f, 100.0f}, {dim.width - 80.0f, 70.f}, 
	// 					{dim.width - 40.0f, 70.0f}, {dim.width - 10.0f, 100.0f}};
	// canvas->drawConvexPolygon(points, 4, paint);

	// // add cloud
	// color = GColor::RGBA(1.0f, 1.0f, 1.0f, 0.85f);
	// r = GRect::LTRB(25.0f, 65.0f, 85.0f, 90.0f);
	// canvas->fillRect(r, color);

	// // add cloud
	// color = GColor::RGBA(1.0f, 1.0f, 1.0f, 0.85f);
	// r = GRect::LTRB(85.0f, 10.0f, 145.0f, 35.0f);
	// canvas->fillRect(r, color);

	// // add wave effect
	// color = GColor::RGBA(0.0f, 0.4f, 1.0f, 1.0f);
	// r = GRect::LTRB(15.0f, dim.height - 60.0f, 35.0f, dim.height - 50.0f);
	// canvas->fillRect(r, color);
	// r = GRect::LTRB(65.0f, dim.height - 40.0f, 85.0f, dim.height - 30.0f);
	// canvas->fillRect(r, color);
	// r = GRect::LTRB(115.0f, dim.height - 60.0f, 135.0f, dim.height - 50.0f);
	// canvas->fillRect(r, color);
	// r = GRect::LTRB(205.0f, dim.height - 20.0f, 225.0f, dim.height - 10.0f);
	// canvas->fillRect(r, color);
	// r = GRect::LTRB(150.0f, dim.height - 30.0f, 170.0f, dim.height - 20.0f);
	// canvas->fillRect(r, color);

    return "Convex Island";
}
*/

std::string GDrawSomething(GCanvas* canvas, GISize dim) {
	// // paint the sky
	GColor color = GColor::RGBA(0.0f, 0.7f, 1.0f, 1.0f);
	GRect r = GRect::LTRB(0.0f, 0.0f, dim.width, dim.height - 120.0f);
	canvas->fillRect(r, color);

	// add left cloud
	color = GColor::RGBA(1.0f, 1.0f, 1.0f, 0.85f);
	GPoint cloudPoints[] = {
		{90.0f, 5.0f}, {140.0f, 5.0f},
		{145.0f, 10.0f}, {145.0f, 35.0f}, {140.0f, 40.0f}, {90.0f, 40.0f}, 
		{85.0f, 35.0f}, {85.0f, 10.0f}
	};
	GPaint paint = GPaint(color);
	GMatrix m = GMatrix(1.0f, -0.3f, -10.0f, 0.0f, 1.0f, 45.0f);
	canvas->save();
	canvas->concat(m);
	canvas->drawConvexPolygon(cloudPoints, 8, paint);
	canvas->restore();
	
	// add right cloud
	m = GMatrix(1.0f, -0.3f, 90.0f, 0.0f, 1.0f, 10.0f);
	canvas->save();
	canvas->concat(m);
	canvas->drawConvexPolygon(cloudPoints, 8, paint);
	canvas->restore();

	// make the grass
	r = GRect::LTRB(0.0f, dim.height - 120.0f, dim.width, dim.height);
	color = GColor::RGBA(0.3f, 0.8f, 0.3f, 1.0f);
	canvas->fillRect(r, color);

	// add grass patches
	color = GColor::RGBA(0.2f, 0.7f, 0.3f, 1.0f);
	r = GRect::LTRB(15.0f, dim.height - 60.0f, 35.0f, dim.height - 50.0f);
	canvas->fillRect(r, color);

	// middle grass patch
	m = GMatrix::Translate(120.0f, 20.0f);
	canvas->save();
	canvas->concat(m);
	canvas->fillRect(r, color);
	canvas->restore();

	// right top grass patch
	m = GMatrix::Translate(190.0f, -10.0f);
	canvas->save();
	canvas->concat(m);
	canvas->fillRect(r, color);
	canvas->restore();

	// sun
	color = GColor::RGBA(1.0f, 1.0f, 0.0f, 1.0f);
	r = GRect::LTRB(0.0f, 0.0f, 50.0f, 50.0f);
	canvas->fillRect(r, color);

	// add tree trunk
	GColor trunkColor = GColor::RGBA(0.2f, 0.2f, 0.2f, 1.0f);
	r = GRect::LTRB(136.0f, 100.0f, 156.0f, 156.0f);

	// add tree leaves
	GColor leafColor = GColor::RGBA(0.0f, 0.5f, 0.0f, 1.0f);
	paint = GPaint(leafColor);
	GPoint leafPoints[] = {{96.0f, 100.0f}, {126, 70.f}, 
						{166.0f, 70.0f}, {196.0f, 100.0f}};
	GPixel trunkPixel = makePixelFromColor(trunkColor);
	GPixel leafPixel = makePixelFromColor(leafColor);

	// first tree
	m = GMatrix(1.0f, -0.5f, -40.0f, 0.0f, 1.0f, 0.0f);
	canvas->save();
	canvas->concat(m);
	auto sh = CreateTreeShader(trunkPixel, leafPixel, 255.0f, 255.0f);
	paint.setShader(sh);
	canvas->drawConvexPolygon(leafPoints, 4, paint);
	canvas->drawRect(r, paint);

	// second tree
	m = GMatrix(1.0f, -0.5f, 100.0f, 0.0f, 1.0f, 0.0f);
	canvas->restore();
	canvas->save();
	canvas->concat(m);
	canvas->drawConvexPolygon(leafPoints, 4, paint);
	canvas->drawRect(r, paint);
	canvas->restore();

	// third tree
	m = GMatrix(1.0f, -0.5f, 10.0f, 0.0f, 1.0f, 50.0f);
	canvas->save();
	canvas->concat(m);
	canvas->drawConvexPolygon(leafPoints, 4, paint);
	canvas->drawRect(r, paint);
	canvas->restore();
	return "Trees in the wind";
}

// PA3
void MyCanvas::save() {
	matrix_stack.push_back(ctm);
}

void MyCanvas::restore() {
	ctm = matrix_stack.back();
	matrix_stack.pop_back();
}

void MyCanvas::concat(const GMatrix& matrix) {
	ctm = ctm * matrix;
}


// PA6
// void drawTriangleWithTex(const GPoint pts[3], const GPoint tex[3], GShader* originalShader) {
//     GMatrix P, T, invT;
//     P = computeBasis(pts[0], pts[1], pts[2]);
//     T = computeBasis(tex[0], tex[1], tex[2]);

//     ProxyShader proxy(originalShader, P * T-1);
//     GPaint p(&proxy);

//     this->drawTriangle(pts, p);
// }

// class ProxyShader : public GShader {
//     GShader* fRealShader;
//     GMatrix  fExtraTransform;
// public:
//     ProxyShader(GShader* shader, const GMatrix& extraTransform)
//         : fRealShader(shader), fExtraTransform(extraTransform) {}

//     bool isOpaque() override { return fRealShader>isOpaque(); }

//     bool setContext(const GMatrix& ctm) override {
//         return fRealShader>setContext(ctm * fExtraTransform);
//     }
    
//     void shadeRow(int x, int y, int count, GPixel row[]) override {
//         fRealShader>shadeRow(x, y, count, row);
//     }
// };

void MyCanvas::drawMeshColors(const GPoint verts[], const GColor colors[], int count, const int indices[]) {
	int n = 0;
	GPoint p0, p1, p2;
	GColor c0, c1, c2;
	std::shared_ptr<GShader> sh;
	GPaint p;
	for (int i=0; i<count; i++) {
		p0 = verts[indices[n]];
		p1 = verts[indices[n+1]];
		p2 = verts[indices[n+2]];
		c0 = colors[indices[n]];
		c1 = colors[indices[n+1]];
		c2 = colors[indices[n+2]];
		sh = CreateTriColorShader(p0, p1, p2, c0, c1, c2);
		p.setShader(sh);
		GPoint poly[] = {p0, p1, p2};
		this->drawConvexPolygon(poly, 3, p);
		n += 3;
	}	
}

void MyCanvas::drawMeshTexs(const GPoint verts[], const GPoint texs[], int count, const int indices[], const GPaint& paint) {
	int n = 0;
	GPoint p0, p1, p2;
	GPoint tex0, tex1, tex2;
	GMatrix P, T, invT;
	std::shared_ptr<GShader> sh;
	GPaint texPaint = paint;
	GPaint p;
	for (int i=0; i<count; i++) {
		p0 = verts[indices[n]];
		p1 = verts[indices[n+1]];
		p2 = verts[indices[n+2]];
		tex0 = texs[indices[n]];
		tex1 = texs[indices[n+1]];
		tex2 = texs[indices[n+2]];
		
		P = computeBases(p0, p1, p2);
		T = computeBases(tex0, tex1, tex2);
		auto invTPtr = T.invert();
		if (invTPtr)
			invT = *invTPtr;
		else
			assert(false);
		sh = CreateProxyShader(texPaint.peekShader(), P*invT);
		p.setShader(sh);
		GPoint poly[] = {p0, p1, p2};
		this->drawConvexPolygon(poly, 3, p);
		n += 3;
	}	
}

void MyCanvas::drawMeshColorsAndTexs(const GPoint verts[], const GColor colors[], const GPoint texs[],
									int count, const int indices[], const GPaint& paint) {
	int n = 0;
	GPoint p0, p1, p2;
	GColor c0, c1, c2;
	GPoint tex0, tex1, tex2;
	GMatrix P, T, invT;
	std::shared_ptr<GShader> colorShader;
	std::shared_ptr<GShader> texShader;
	std::shared_ptr<GShader> doubleShader;
	GPaint texPaint = paint;
	GPaint p;
	for (int i=0; i<count; i++) {
		p0 = verts[indices[n]];
		p1 = verts[indices[n+1]];
		p2 = verts[indices[n+2]];

		// make color shader
		c0 = colors[indices[n]];
		c1 = colors[indices[n+1]];
		c2 = colors[indices[n+2]];
		colorShader = CreateTriColorShader(p0, p1, p2, c0, c1, c2);

		// make texture shader
		tex0 = texs[indices[n]];
		tex1 = texs[indices[n+1]];
		tex2 = texs[indices[n+2]];		
		P = computeBases(p0, p1, p2);
		T = computeBases(tex0, tex1, tex2);
		auto invTPtr = T.invert();
		if (invTPtr)
			invT = *invTPtr;
		else
			assert(false);
		texShader = CreateProxyShader(texPaint.peekShader(), P*invT);

		// make double shader
		doubleShader = CreateDoubleShader(colorShader.get(), texShader.get());
		p.setShader(doubleShader);
		GPoint poly[] = {p0, p1, p2};
		this->drawConvexPolygon(poly, 3, p);
		n += 3;
	}	
}

void MyCanvas::drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[],
						int count, const int indices[], const GPaint& paint) {
	bool usingColors = colors != nullptr;
	bool usingTexs = texs != nullptr;
	GPaint p = paint;
	if (usingColors && !usingTexs)
		drawMeshColors(verts, colors, count, indices);
	else if (!usingColors && usingTexs)
		drawMeshTexs(verts, texs, count, indices, paint);
	else if (usingColors && usingTexs)
		drawMeshColorsAndTexs(verts, colors, texs, count, indices, paint);
}

/*
void MyCanvas::drawQuadLevel0(const GPoint verts[4], const GColor colors[4], const GPoint texs[4], const GPaint& paint) {
	const int indices[] = {
		0, 1, 3,
		1, 2, 3
	};
	bool usingColors = colors != nullptr;
	bool usingTexs = texs != nullptr;
	if (usingColors && !usingTexs)
		drawMeshColors(verts, colors, 2, indices);
	else if (!usingColors && usingTexs)
		drawMeshTexs(verts, texs, 2, indices, paint);
	else if (usingColors && usingTexs)
		drawMeshColorsAndTexs(verts, colors, texs, 2, indices, paint);
}

void MyCanvas::drawQuadLevel1(const GPoint verts[4], const GColor colors[4], const GPoint texs[4], const GPaint& paint) {
	bool usingColors = colors != nullptr;
	bool usingTexs = texs != nullptr;
	GColor newColors[9];
	GPoint newTexs[9];
	GColor ca, cb, cc, cd;
	GPoint ta, tb, tc, td;

	// set indices for each triangle
	const int indices[] = {
		0, 1, 2,
		1, 3, 2,
		2, 3, 6,
		3, 7, 6,
		1, 4, 3,
		4, 5, 3,
		3, 5, 7,
		5, 8, 7
	};

	// calculate new vertices
	GPoint newVerts[9];
	ta = verts[0];
	tb = verts[1];
	tc = verts[2];
	td = verts[3];
	newVerts[0] = ta;
	newVerts[1] = 0.5*(ta + tb);
	newVerts[2] = 0.5*(ta + td);
	newVerts[3] = 0.25*(ta + tb + tc + td);
	newVerts[4] = tb;
	newVerts[5] = 0.5*(tb + tc);
	newVerts[6] = td;
	newVerts[7] = 0.5*(td + tc);
	newVerts[8] = tc;

	if (usingColors) {
		ca = colors[0];
		cb = colors[1];
		cc = colors[2];
		cd = colors[3];
		newColors[0] = ca;
		newColors[1] = 0.5*(ca + cb);
		newColors[2] = 0.5*(ca + cd);
		newColors[3] = 0.25*(ca + cb + cc + cd);
		newColors[4] = cb;
		newColors[5] = 0.5*(cb + cc);
		newColors[6] = cd;
		newColors[7] = 0.5*(cd + cc);
		newColors[8] = cc;
	}

	if (usingTexs) {
		ta = texs[0];
		tb = texs[1];
		tc = texs[2];
		td = texs[3];
		newTexs[0] = ta;
		newTexs[1] = 0.5*(ta + tb);
		newTexs[2] = 0.5*(ta + td);
		newTexs[3] = 0.25*(ta + tb + tc + td);
		newTexs[4] = tb;
		newTexs[5] = 0.5*(tb + tc);
		newTexs[6] = td;
		newTexs[7] = 0.5*(td + tc);
		newTexs[8] = tc;
	}

	if (usingColors && !usingTexs) {
		drawMeshColors(newVerts, newColors, 8, indices);
	} else if (!usingColors && usingTexs) {
		drawMeshTexs(newVerts, newTexs, 8, indices, paint);
	} else if (usingColors && usingTexs) {
		drawMeshColorsAndTexs(newVerts, newColors, newTexs, 8, indices, paint);
	}

}

// void MyCanvas::drawQuadLevel2() {}
*/
void MyCanvas::drawQuadColors(const GPoint verts[4], const GColor colors[4], int level) {
	int totalLines = level + 2;
	int numVertices = totalLines * totalLines;
	GPoint newVerts[numVertices];
	GColor newColors[numVertices];
	GColor a = colors[0];
	GColor b = colors[1];
	GColor c = colors[2];
	GColor d = colors[3];
	int denom = level + 1;
	int numTriangles = denom*denom*2;
	int indices[numTriangles*3];

	float u = 0.0f;
	float v = 0.0f;
	float fi=0.0f;
	float fj=0.0f;
	for (int i=0; i<totalLines; i++) {
		v = fi / denom;
		for (int j=0; j<totalLines; j++) {
			u = fj / denom;
			// get color for each vertex
			newColors[i*totalLines + j] = bilinearColor(u, v, a, b, c, d);
			// fill in vertex
			newVerts[i*totalLines + j] = bilinearPoint(u, v, verts[0], verts[1], verts[2], verts[3]);
			fj += 1.0f;
		}
		fj = 0.0f;
		fi += 1.0f;	
	}
	// set indices
	int numTrisPerRow = denom * 2;
	int numRows = denom;
	int numPairsPerRow = denom;
	for (int r=0; r<numRows; r++) {
		// fill in first triangle pair for every row
		int rowIdx = r * numTrisPerRow * 3;
		indices[rowIdx] = r * totalLines;
		indices[rowIdx + 1] = r * totalLines + 1;
		indices[rowIdx + 2] = (r + 1) * totalLines;
		// second triangle in pair
		indices[rowIdx + 3] = indices[rowIdx + 1];
		indices[rowIdx + 4] = indices[rowIdx + 2];
		indices[rowIdx + 5] = indices[rowIdx + 4] + 1;
		// fill in rest of row
		for (int p=1; p<numPairsPerRow; p++) {
			int curPair = 6 * p + rowIdx;
			int prevPair = 6 * (p - 1) + rowIdx;
			// first triangle in pair
			indices[curPair] = indices[prevPair] + 1;
			indices[curPair + 1] = indices[prevPair + 1] + 1;
			indices[curPair + 2] = indices[prevPair + 2] + 1;
			// second triangle in pair
			indices[curPair + 3] = indices[prevPair + 3] + 1;
			indices[curPair + 4] = indices[prevPair + 4] + 1;
			indices[curPair + 5] = indices[prevPair + 5] + 1;
		}
	}
	drawMeshColors(newVerts, newColors, numTriangles, indices);
}

void MyCanvas::drawQuadTexs(const GPoint verts[4], const GPoint texs[4], int level, const GPaint& paint) {
	int totalLines = level + 2;
	int numVertices = totalLines * totalLines;
	GPoint newVerts[numVertices];
	GPoint newTexs[numVertices];
	GPoint a = texs[0];
	GPoint b = texs[1];
	GPoint c = texs[2];
	GPoint d = texs[3];
	int denom = level + 1;
	int numTriangles = denom*denom*2;
	int indices[numTriangles*3];

	float u = 0.0f;
	float v = 0.0f;
	float fi=0.0f;
	float fj=0.0f;
	for (int i=0; i<totalLines; i++) {
		v = fi / denom;
		for (int j=0; j<totalLines; j++) {
			u = fj / denom;
			// get shader tex for each vertex
			newTexs[i*totalLines + j] = bilinearPoint(u, v, a, b, c, d);
			// fill in vertex
			newVerts[i*totalLines + j] = bilinearPoint(u, v, verts[0], verts[1], verts[2], verts[3]);
			fj += 1.0f;
		}
		fj = 0.0f;
		fi += 1.0f;	
	}
	// set indices
	int numTrisPerRow = denom * 2;
	int numRows = denom;
	int numPairsPerRow = denom;
	for (int r=0; r<numRows; r++) {
		// fill in first triangle pair for every row
		int rowIdx = r * numTrisPerRow * 3;
		indices[rowIdx] = r * totalLines;
		indices[rowIdx + 1] = r * totalLines + 1;
		indices[rowIdx + 2] = (r + 1) * totalLines;
		// second triangle in pair
		indices[rowIdx + 3] = indices[rowIdx + 1];
		indices[rowIdx + 4] = indices[rowIdx + 2];
		indices[rowIdx + 5] = indices[rowIdx + 4] + 1;
		// fill in rest of row
		for (int p=1; p<numPairsPerRow; p++) {
			int curPair = 6 * p + rowIdx;
			int prevPair = 6 * (p - 1) + rowIdx;
			// first triangle in pair
			indices[curPair] = indices[prevPair] + 1;
			indices[curPair + 1] = indices[prevPair + 1] + 1;
			indices[curPair + 2] = indices[prevPair + 2] + 1;
			// second triangle in pair
			indices[curPair + 3] = indices[prevPair + 3] + 1;
			indices[curPair + 4] = indices[prevPair + 4] + 1;
			indices[curPair + 5] = indices[prevPair + 5] + 1;
		}
	}
	drawMeshTexs(newVerts, newTexs, numTriangles, indices, paint);
}

void MyCanvas::drawQuadColorsAndTexs(const GPoint verts[4], const GColor colors[4], const GPoint texs[4], int level, const GPaint& paint) {
	int totalLines = level + 2;
	int numVertices = totalLines * totalLines;
	GPoint newVerts[numVertices];
	GPoint newTexs[numVertices];
	GColor newColors[numVertices];
	GPoint ta = texs[0];
	GPoint tb = texs[1];
	GPoint tc = texs[2];
	GPoint td = texs[3];
	GColor ca = colors[0];
	GColor cb = colors[1];
	GColor cc = colors[2];
	GColor cd = colors[3];
	int denom = level + 1;
	int numTriangles = denom*denom*2;
	int indices[numTriangles*3];

	float u = 0.0f;
	float v = 0.0f;
	float fi=0.0f;
	float fj=0.0f;
	for (int i=0; i<totalLines; i++) {
		v = fi / denom;
		for (int j=0; j<totalLines; j++) {
			u = fj / denom;
			// get color for each vertex
			newColors[i*totalLines + j] = bilinearColor(u, v, ca, cb, cc, cd);
			// get shader tex for each vertex
			newTexs[i*totalLines + j] = bilinearPoint(u, v, ta, tb, tc, td);
			// fill in vertex
			newVerts[i*totalLines + j] = bilinearPoint(u, v, verts[0], verts[1], verts[2], verts[3]);
			fj += 1.0f;
		}
		fj = 0.0f;
		fi += 1.0f;	
	}
	// set indices
	int numTrisPerRow = denom * 2;
	int numRows = denom;
	int numPairsPerRow = denom;
	for (int r=0; r<numRows; r++) {
		// fill in first triangle pair for every row
		int rowIdx = r * numTrisPerRow * 3;
		indices[rowIdx] = r * totalLines;
		indices[rowIdx + 1] = r * totalLines + 1;
		indices[rowIdx + 2] = (r + 1) * totalLines;
		// second triangle in pair
		indices[rowIdx + 3] = indices[rowIdx + 1];
		indices[rowIdx + 4] = indices[rowIdx + 2];
		indices[rowIdx + 5] = indices[rowIdx + 4] + 1;
		// fill in rest of row
		for (int p=1; p<numPairsPerRow; p++) {
			int curPair = 6 * p + rowIdx;
			int prevPair = 6 * (p - 1) + rowIdx;
			// first triangle in pair
			indices[curPair] = indices[prevPair] + 1;
			indices[curPair + 1] = indices[prevPair + 1] + 1;
			indices[curPair + 2] = indices[prevPair + 2] + 1;
			// second triangle in pair
			indices[curPair + 3] = indices[prevPair + 3] + 1;
			indices[curPair + 4] = indices[prevPair + 4] + 1;
			indices[curPair + 5] = indices[prevPair + 5] + 1;
		}
	}
	drawMeshColorsAndTexs(newVerts, newColors, newTexs, numTriangles, indices, paint);
}

void MyCanvas::drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4],
						int level, const GPaint& paint) {
	bool usingColors = colors != nullptr;
	bool usingTexs = texs != nullptr;
	if (usingColors && !usingTexs)
		drawQuadColors(verts, colors, level);
	else if (!usingColors && usingTexs)
		drawQuadTexs(verts, texs, level, paint);
	else if (usingColors && usingTexs)
		drawQuadColorsAndTexs(verts, colors, texs, level, paint);
}