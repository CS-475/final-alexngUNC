/*
 *  Copyright 2024 <me>
 */

#ifndef _g_starter_canvas_h_
#define _g_starter_canvas_h_

#include "include/GCanvas.h"
#include "include/GRect.h"
#include "include/GColor.h"
#include "include/GBitmap.h"
#include "include/GMatrix.h"
#include "include/GPath.h"
#include "include/GPathBuilder.h"

class MyCanvas : public GCanvas {
public:
    MyCanvas(const GBitmap& device) : fDevice(device), ctm(GMatrix()) {}

	void save() override;
	void restore() override;
	void concat(const GMatrix& matrix) override;
    void clear(const GColor& color) override;
	void drawRect(const GRect&, const GPaint&) override;
	void drawConvexPolygon(const GPoint[], int count, const GPaint&) override;
	void drawPath(const GPath&, const GPaint&) override;
	void drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[],
							int count, const int indices[], const GPaint&) override;
	void drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4],
                        	int level, const GPaint&) override;
	
	// Mine
	void drawMeshColors(const GPoint verts[], const GColor colors[], int count, const int indices[]);
	void drawMeshTexs(const GPoint verts[], const GPoint texs[], int count, const int indices[], const GPaint&);
	void drawMeshColorsAndTexs(const GPoint verts[], const GColor colors[], const GPoint texs[],
							int count, const int indices[], const GPaint&);
	void drawQuadColors(const GPoint verts[4], const GColor colors[4], int level);
	void drawQuadTexs(const GPoint verts[4], const GPoint texs[4], int level, const GPaint&);
	void drawQuadColorsAndTexs(const GPoint verts[4], const GColor colors[4], const GPoint texs[4], int level, const GPaint&);
	
private:
    // Note: we store a copy of the bitmap
    const GBitmap fDevice;
	std::vector<GMatrix> matrix_stack;
	GMatrix ctm;
};

#endif
