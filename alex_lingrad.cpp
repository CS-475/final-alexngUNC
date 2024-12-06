#include "alex_lingrad.h"
#include "alex_lingrad1.h"
#include "alex_lingrad2.h"

std::shared_ptr<GShader> GCreateLinearGradient(GPoint p0, GPoint p1, const GColor colors[], int count, GTileMode mode) {
	if (count < 1) return nullptr;
	else if (count == 1) return std::make_shared<MyLinearGradient1>(p0, p1, colors[0]);
	else if (count == 2) return std::make_shared<MyLinearGradient2>(p0, p1, colors[0], colors[1], mode);
	return std::make_shared<MyLinearGradient>(p0, p1, colors, count, mode);
}
