#ifndef INITIALIZER_VISITOR_HPP
#define INITIALIZER_VISITOR_HPP

class CubeInitializer;
class SphereInitializer;

class IInitializerVisitor {
public:
    virtual ~IInitializerVisitor() = default;

    virtual void visit(CubeInitializer& initializer) = 0;
    virtual void visit(SphereInitializer& initializer) = 0;
};

#endif