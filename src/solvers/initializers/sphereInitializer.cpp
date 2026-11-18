#include "sphereInitializer.hpp"

using namespace glm;
using namespace std;

void SphereInitializer::loadShader(){
    ShaderProgram::loadCompute(m_gridFillShader, "src/shaders/initializer/sphereinitializer.glsl");
}

void SphereInitializer::setRadiusAndPos(float radius, vec3 pos, float scale){
    float clampedScale = glm::max(scale, 1e-8f);
    if (m_min != (pos - vec3(radius)) / clampedScale || m_max != (pos + vec3(radius)) / clampedScale)
        m_hasChanged = true;
    m_min = (pos - vec3(radius)) / clampedScale;
    m_max = (pos + vec3(radius)) / clampedScale;
}

bool SphereInitializer::isInInitializer(glm::vec3 position, float scale) {
    scale = std::max(scale, 1e-5f);
    position /= scale;
    return length(position - pos(1.0f)) <= getRadius(1.0f);
}