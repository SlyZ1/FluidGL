#ifndef FLIP_SOLVER_CPU_CONFIG
#define FLIP_SOLVER_CPU_CONFIG
 
#include "particleSolverConfig.hpp"

#include <glm/glm.hpp>

class FlipSolverCPUConfig : public IParticleSolverConfig {
protected:
    float m_partPerH = 2;
    float m_overrelaxation = 1.9f;
    glm::vec2 m_domainSize = glm::vec2(200.0f, 200.0f);

public:
    FlipSolverCPUConfig() : IParticleSolverConfig() {}
    FlipSolverCPUConfig(int partN, float partRadius, float dt, float partPerH, int overrelaxation, glm::vec2 domainSize) 
    : IParticleSolverConfig(partN, partRadius, dt), m_partPerH(partPerH), m_overrelaxation(overrelaxation), m_domainSize(domainSize) {}
    ~FlipSolverCPUConfig() override = default;

    std::unique_ptr<ISolverConfig> clone() const override; 

    float getH() const { return 2 * m_partRadius * m_partPerH; }
    float getDensity() const { return m_partPerH * m_partPerH; }
    
    float getPartPerH() const { return m_partPerH; }
    void setPartPerH(float partPerH) { m_partPerH = std::max(partPerH, 1.0f); }
    
    float getOverrelaxation() const { return m_overrelaxation; }
    void setOverrelaxation(float overrelaxation) { m_overrelaxation = glm::clamp(overrelaxation, 1.0f, 2.0f); }

    glm::vec2 getDomainSize() const { return m_domainSize; }
    void setDomainSize(glm::vec2 domainSize) { m_domainSize = glm::max(domainSize, 0.f); }

    glm::ivec2 getGridDim() const {
        float h = getH();
        return glm::ivec2(
            glm::max(1, (int)glm::floor(m_domainSize.x / h)),
            glm::max(1, (int)glm::floor(m_domainSize.y / h))
        );
    }
    int getGridX() const { return getGridDim().x; } 
    int getGridY() const { return getGridDim().y; }
};

#endif