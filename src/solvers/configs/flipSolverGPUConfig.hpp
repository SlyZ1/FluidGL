#ifndef FLIP_SOLVER_GPU_CONFIG
#define FLIP_SOLVER_GPU_CONFIG

#include "particleSolverConfig.hpp"
#include "solvers/fluidInitializer.hpp"

#include <glm/glm.hpp>

class FlipSolverGPUConfig : public IParticleSolverConfig {
protected:
    float m_flipRatio = 0.9f;
    float m_partPerH = 2;
    float m_densityMultiplier = 5;
    glm::vec3 m_domainSize = glm::vec3(10.0f);
    float m_sigma = 0;
    float m_gravity = 9.81f;
    int m_cgMaxIter = 20;
    float m_cgTol = 1e-3f;
    FluidInitializer m_fluidInitializer = {};

public:
    FlipSolverGPUConfig() : IParticleSolverConfig() {}
    FlipSolverGPUConfig(int partN, float partRadius, float dt, float flipRatio, float partPerH, float densityMultiplier, glm::vec3 domainSize) 
    : IParticleSolverConfig(partN, partRadius, dt), m_flipRatio(flipRatio), m_partPerH(partPerH), m_densityMultiplier(densityMultiplier), m_domainSize(domainSize) {}
    ~FlipSolverGPUConfig() override = default;

    std::unique_ptr<ISolverConfig> clone() const override; 

    float getH() const { return 2 * m_partRadius * m_partPerH; }
    float getDensity() const { return m_partPerH * m_partPerH * m_partPerH; }
    
    float getDensityMultiplier() const { return m_densityMultiplier; }
    void setDensityMultiplier(float densityMultiplier) { m_densityMultiplier = std::max(densityMultiplier, 1.0f); }
    
    float getFlipRatio() const { return m_flipRatio; }
    void setFlipRatio(float flipRatio) { m_flipRatio = glm::clamp(flipRatio, 0.0f, 1.0f); }

    float getSigma() const { return m_sigma; }
    void setSigma(float sigma) { m_sigma = std::max(sigma, 0.0f); }

    float getGravity() const { return m_gravity; }
    void setGravity(float gravity) { m_gravity = std::max(gravity, 0.0f); }

    int getCgMaxIter() const { return m_cgMaxIter; }
    void setCgMaxIter(int cgMaxIter) { m_cgMaxIter = cgMaxIter; }

    float getCgTol() const { return m_cgTol; }
    void setCgTol(float cgTol) { m_cgTol = cgTol; }

    FluidInitializer getFluidInitializer() const { return m_fluidInitializer; }
    void setFluidInitializer(FluidInitializer fluidInitializer) { m_fluidInitializer = fluidInitializer; }

    float getPartPerH() const { return m_partPerH; }
    void setPartPerH(float partPerH) { m_partPerH = std::max(partPerH, 1.0f); }

    glm::vec3 getDomainSize() const { return m_domainSize; }
    void setDomainSize(glm::vec3 domainSize) { m_domainSize = glm::max(domainSize, 0.f); }


    glm::ivec3 getGridDim() const {
        float h = getH();
        return glm::ivec3(
            glm::max(1, (int)glm::floor(m_domainSize.x / h)),
            glm::max(1, (int)glm::floor(m_domainSize.y / h)),
            glm::max(1, (int)glm::floor(m_domainSize.z / h))
        );
    }
    int getGridX() const { return getGridDim().x; } 
    int getGridY() const { return getGridDim().y; }
    int getGridZ() const { return getGridDim().z; }
};

#endif