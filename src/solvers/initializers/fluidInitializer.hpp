#ifndef FLUID_INITIALIZER_HPP
#define FLUID_INITIALIZER_HPP

#include <glm/glm.hpp>

#include "core/shader_program.hpp"
#include "initializerVisitor.hpp"

class FluidInitializer {
protected:
    glm::vec3 m_min = glm::vec3(0.0f);
    glm::vec3 m_max = glm::vec3(0.0f);
    glm::ivec3 m_gridDim = glm::ivec3(0);

    bool m_hasChanged = false;

    GLuint m_gridBuffer = 0;

    ShaderProgram m_gridFillShader = {};

public:
    FluidInitializer();
    FluidInitializer(glm::vec3 min, glm::vec3 max, float scale);

    ~FluidInitializer() = default;

    virtual bool operator==(const FluidInitializer& other) const {
        return m_min==other.min(1) && m_max==other.max(1);
    }

    void loadOpenGl(float h);

    virtual bool isInInitializer(glm::vec3 position, float scale) = 0;
    virtual void accept(IInitializerVisitor& visitor) = 0;

    void setScale(float scale);
    glm::vec3 pos(float scale) const { return (m_min + m_max) * 0.5f * scale; }
    glm::vec3 bounds(float scale) const { return abs((m_max - m_min) * scale); }
    glm::vec3 min(float scale) const { return m_min * scale; }
    glm::vec3 max(float scale) const { return m_max * scale; }
    GLuint getGridBuffer() const { return m_gridBuffer; }
    glm::ivec3 getGridDim() const { return m_gridDim; }
    bool hasChanged() const { return m_hasChanged; }
};

#endif 