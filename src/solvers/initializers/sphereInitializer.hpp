#ifndef SPHERE_INITIALIZER_HPP
#define SPHERE_INITIALIZER_HPP

#include "fluidInitializer.hpp"

class SphereInitializer : public FluidInitializer {
private:
    void loadShader();

public:
    SphereInitializer() : FluidInitializer() { loadShader(); }
    SphereInitializer(glm::vec3 min, glm::vec3 max, float scale) 
    : FluidInitializer(min, max, scale) { loadShader(); }

    ~SphereInitializer() = default;

    float getRadius(float scale) const { return (m_max.x - m_min.x) * 0.5f * scale; }
    void setRadiusAndPos(float radius, glm::vec3 pos, float scale);

    bool isInInitializer(glm::vec3 position, float scale) override;

    void accept(IInitializerVisitor& visitor) override { visitor.visit(*this); }
};

#endif