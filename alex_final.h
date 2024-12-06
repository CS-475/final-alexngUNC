#ifndef alex_final_DEFINED
#define alex_final_DEFINED

#include "include/GFinal.h"
#include "alex_linear_pos.h"
#include "alex_color_matrix_shader.h"

class MyFinal : public GFinal {
public:
	std::shared_ptr<GShader> createLinearPosGradient(GPoint p0, GPoint p1,
                                                    const GColor colors[],
													const float pos[],
													int count)
	{
		return std::make_shared<LinearPosGradient>(p0, p1, colors, pos, count);
	}

	std::shared_ptr<GShader> createColorMatrixShader(const GColorMatrix& matrix, GShader* realShader) {
        return std::make_shared<MyColorMatrixShader>(matrix, realShader);
    }
};

std::unique_ptr<GFinal> GCreateFinal() {
	return std::make_unique<MyFinal>();
}

#endif