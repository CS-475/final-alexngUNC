#ifndef alex_proxy_shader_DEFINED
#define alex_proxy_shader_DEFINED

#include "include/GShader.h"

class ProxyShader : public GShader {
    GShader* fRealShader;
    GMatrix  fExtraTransform;
public:
    ProxyShader(GShader* shader, const GMatrix& extraTransform)
        : fRealShader(shader), fExtraTransform(extraTransform) {}

    bool isOpaque() override { return fRealShader->isOpaque(); }

    bool setContext(const GMatrix& ctm) override {
		return fRealShader->setContext(ctm * fExtraTransform);
    }
    
    void shadeRow(int x, int y, int count, GPixel row[]) override {
        fRealShader->shadeRow(x, y, count, row);
    }
};

std::shared_ptr<GShader> CreateProxyShader(GShader* shader, const GMatrix& extraTransform) {
	return std::make_shared<ProxyShader>(shader, extraTransform);
}

#endif