#include "cubeInitializer.hpp"

using namespace glm;
using namespace std;

void CubeInitializer::loadShader(){
    ShaderProgram::loadCompute(m_gridFillShader, "src/shaders/initializer/cubeinitializer.glsl");
}

void CubeInitializer::setSizeAndPos(vec3 size, vec3 pos, float scale){
    float clampedScale = glm::max(scale, 1e-8f);
    if (m_min != (pos - size * 0.5f) / clampedScale || m_max != (pos + size * 0.5f) / clampedScale)
        m_hasChanged = true;
    m_min = (pos - size * 0.5f) / clampedScale;
    m_max = (pos + size * 0.5f) / clampedScale;
}