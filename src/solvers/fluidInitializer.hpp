#ifndef FLUID_INITIALIZER_HPP
#define FLUID_INITIALIZER_HPP

#include <glm/glm.hpp>

struct FluidInitializer {
private:
    glm::vec3 m_min = glm::vec3(0.0f);
    glm::vec3 m_max = glm::vec3(0.0f);

public:
    
    bool operator==(const FluidInitializer& other) const {
        return m_min==other.min(1) && m_max==other.max(1);
    }

    FluidInitializer() {
        m_min = glm::vec3(0.0f);
        m_max = glm::vec3(0.0f);
    }

    FluidInitializer(glm::vec3 min, glm::vec3 max, float scale) 
    {
        float clampedScale = glm::max(scale, 1e-8f);
        m_min = min / clampedScale;
        m_max = max / clampedScale;
    }

    glm::vec3 pos(float scale) const {
        return (m_min + m_max) * 0.5f * scale;
    }

    glm::vec3 bounds(float scale) const {
        return abs((m_max - m_min) * scale);
    }

    glm::vec3 min(float scale) const {
        return m_min * scale;
    }

    glm::vec3 max(float scale) const {
        return m_max * scale;
    }
};

#endif 