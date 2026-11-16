#ifndef RENDERER_VISITOR_HPP
#define RENDERER_VISITOR_HPP

class TriangleRenderer;
class SmokeRenderer3D;
class LiquidRenderer3D;
class SmokeRayMarcher;

class IRendererVisitor {
public:
    virtual ~IRendererVisitor() = default;

    virtual void visit(TriangleRenderer& renderer) = 0;
    virtual void visit(SmokeRenderer3D& renderer) = 0;
    virtual void visit(LiquidRenderer3D& renderer) = 0;
    virtual void visit(SmokeRayMarcher& renderer) = 0;
};

#endif