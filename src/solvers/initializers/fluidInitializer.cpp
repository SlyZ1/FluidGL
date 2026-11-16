#include "fluidInitializer.hpp"

using namespace glm;
using namespace std;

void FluidInitializer::loadOpenGl(float h){
    m_gridDim = ivec3(ceil((m_max - m_min) / h));

    ShaderProgram::deleteBuffer(m_gridBuffer);
    ShaderProgram::genSSBO<uint>(m_gridBuffer, m_gridDim.x * m_gridDim.y * m_gridDim.z, 0.0f);

    m_gridFillShader.reload();
    m_gridFillShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_gridBuffer);
    glUniform3i(ShaderProgram::getVarLoc("gridDim"), m_gridDim.x, m_gridDim.y, m_gridDim.z);
    m_gridFillShader.dispatch((m_gridDim + 7) / 8);
}

FluidInitializer::FluidInitializer() {
    m_min = vec3(0.0f);
    m_max = vec3(0.0f);
}

FluidInitializer::FluidInitializer(vec3 min, vec3 max, float scale) 
{
    float clampedScale = glm::max(scale, 1e-8f);
    m_min = min / clampedScale;
    m_max = max / clampedScale;
}

void FluidInitializer::setScale(float scale) {
    scale = glm::min(scale, 1e-10f);
    m_min /= scale;
    m_max /= scale;
}