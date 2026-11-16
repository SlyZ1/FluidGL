#ifndef RENDERER_UI_HPP
#define RENDERER_UI_HPP

#include "../rendererVisitor.hpp"

class RendererUI : public IRendererVisitor {
public:
    ~RendererUI() override = default;

    void visit(TriangleRenderer& renderer) override;
    void visit(SmokeRenderer3D& renderer) override;
    void visit(LiquidRenderer3D& renderer) override;
    void visit(SmokeRayMarcher& renderer) override;
};

#endif