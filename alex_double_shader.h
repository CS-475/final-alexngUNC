#ifndef alex_double_shader_DEFINED
#define alex_double_shader_DEFINED

#include "include/GShader.h"

class DoubleShader : public GShader {
    GShader* shader1;
	GShader* shader2;
public:
    DoubleShader(GShader* shader1, GShader* shader2)
        : shader1(shader1), shader2(shader2) {}

    bool isOpaque() override {
		return shader1->isOpaque() && shader2->isOpaque();
	}

    bool setContext(const GMatrix& ctm) override {
		return shader1->setContext(ctm) && shader2->setContext(ctm);
    }
    
    void shadeRow(int x, int y, int count, GPixel row[]) override {
		GPixel row1[count];
		GPixel row2[count];
        shader1->shadeRow(x, y, count, row1);
        shader2->shadeRow(x, y, count, row2);
		//TODO: add opaque version
		for (int i=0; i<count; i++)
			row[i] = modulateBlend(row1[i], row2[i]);
			// row[i] = row1[i];
    }
};

std::shared_ptr<GShader> CreateDoubleShader(GShader* shader1, GShader* shader2) {
	return std::make_shared<DoubleShader>(shader1, shader2);
}

#endif