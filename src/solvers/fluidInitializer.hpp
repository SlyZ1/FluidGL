#ifndef FLUID_INITIALIZER_HPP
#define FLUID_INITIALIZER_HPP

#include <glm/glm.hpp>

struct FluidInitializer {
    glm::vec3 min = glm::vec3(0.0f);
    glm::vec3 max = glm::vec3(0.0f);
    
    bool operator==(const FluidInitializer& other) const {
        return min==other.min && max==other.max;
    }

    FluidInitializer() {
        min = glm::vec3(0.0f);
        max = glm::vec3(0.0f);
    }

    FluidInitializer(glm::vec3 min_, glm::vec3 max_) : min(min_), max(max_) {}

    glm::vec3 pos() {
        return (min + max) * 0.5f;
    }

    glm::vec3 bounds() {
        return abs(max - min);
    }
};

#endif 