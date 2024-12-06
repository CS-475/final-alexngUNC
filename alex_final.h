#ifndef alex_final_DEFINED
#define alex_final_DEFINED

#include "include/GFinal.h"
#include "alex_linear_pos.h"
#include "alex_color_matrix_shader.h"
#include "alex_lingrad.h"

class MyFinal : public GFinal {
public:
	std::shared_ptr<GShader> createLinearPosGradient(GPoint p0, GPoint p1,
                                                    const GColor colors[],
													const float pos[],
													int count)
	{
		return std::make_shared<MyLinearGradient>(p0, p1, colors, count, GTileMode::kClamp);
	}

	std::shared_ptr<GShader> createColorMatrixShader(const GColorMatrix& matrix, GShader* realShader) {
        return std::make_shared<MyColorMatrixShader>(matrix, realShader);
    }

	std::shared_ptr<GShader> createSweepGradient(GPoint center, float startRadians,
												const GColor colors[], int count) {
		GPoint p1 = center*2;													
        return std::make_shared<MyLinearGradient>(center, p1, colors, count, GTileMode::kClamp);
    }

};

std::unique_ptr<GFinal> GCreateFinal() {
	return std::make_unique<MyFinal>();
}

#endif