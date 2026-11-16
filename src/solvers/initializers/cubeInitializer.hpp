#ifndef CUBE_INITIALIZER_HPP
#define CUBE_INITIALIZER_HPP

#include "fluidInitializer.hpp"

class CubeInitializer : public FluidInitializer {
private:
    void loadShader();

public:
    CubeInitializer() : FluidInitializer() { loadShader(); }
    CubeInitializer(glm::vec3 min, glm::vec3 max, float scale) 
    : FluidInitializer(min, max, scale) { loadShader(); }

    ~CubeInitializer() = default;

    void setSizeAndPos(glm::vec3 size, glm::vec3 pos, float scale);

    bool isInInitializer(glm::vec3, float) override { return true; };

    void accept(IInitializerVisitor& visitor) override { visitor.visit(*this); }
};

#endif