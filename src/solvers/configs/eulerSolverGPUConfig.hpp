#ifndef EULER_SOLVER_GPU_CONFIG
#define EULER_SOLVER_GPU_CONFIG

#include "particleSolverConfig.hpp"
#include "solvers/fluidInitializer.hpp"

#include <glm/glm.hpp>

class EulerSolverGPUConfig : public ISolverConfig {
protected:
    float m_dt = 0.03f;
    float m_h = 1.5f;
    float m_densityMultiplier = 5;
    glm::vec3 m_domainSize = glm::vec3(900.0f, 400.0f, 300.0f);
    float m_sigma = 0;
    float m_gravity = 9.81f;
    int m_cgMaxIter = 20;
    float m_cgTol = 1e-3f;
    bool m_accuracyFirst = true;
    FluidInitializer m_fluidInitializer = FluidInitializer(
        glm::vec3(-450, -100, -150),
        glm::vec3(-150, 100, 150),
        1.5f
    );

public:
    EulerSolverGPUConfig() : ISolverConfig() {}
    EulerSolverGPUConfig(float dt, float h, float densityMultiplier, glm::vec3 domainSize) 
    : ISolverConfig(), m_dt(dt), m_h(h), m_densityMultiplier(densityMultiplier), m_domainSize(domainSize),
    m_fluidInitializer({ glm::vec3(-450, -100, -150), glm::vec3(-150, 100, 150), m_h }) {}
    ~EulerSolverGPUConfig() override = default;

    std::unique_ptr<ISolverConfig> clone() const override; 

    float getH() const { return m_h; }
    void setH(float h) { m_h = h; }

    float getDt() const { return m_dt; }
    void setDT(float dt) { m_dt = dt; }
    
    float getDensityMultiplier() const { return m_densityMultiplier; }
    void setDensityMultiplier(float densityMultiplier) { m_densityMultiplier = std::max(densityMultiplier, 1.0f); }

    float getSigma() const { return m_sigma; }
    void setSigma(float sigma) { m_sigma = std::max(sigma, 0.0f); }

    float getGravity() const { return m_gravity; }
    void setGravity(float gravity) { m_gravity = std::max(gravity, 0.0f); }

    int getCgMaxIter() const { return m_cgMaxIter; }
    void setCgMaxIter(int cgMaxIter) { m_cgMaxIter = cgMaxIter; }

    float getCgTol() const { return m_cgTol; }
    void setCgTol(float cgTol) { m_cgTol = cgTol; }

    bool getAccuracyFirst() const { return m_accuracyFirst; }
    void setAccuracyFirst(bool accuracyFirst) { m_accuracyFirst = accuracyFirst; }

    FluidInitializer getFluidInitializer() const { return m_fluidInitializer; }
    void setFluidInitializer(FluidInitializer fluidInitializer) { m_fluidInitializer = fluidInitializer; }

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