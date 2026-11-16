#ifndef FLIP_SOLVER_GPU_CONFIG
#define FLIP_SOLVER_GPU_CONFIG

#include "particleSolverConfig.hpp"
#include "solvers/initializers/fluidInitializer.hpp"

class FlipSolverGPUConfig : public IParticleSolverConfig {
protected:
    float m_flipRatio = 0.9f;
    float m_partPerH = 1;
    float m_densityMultiplier = 5;
    glm::vec3 m_domainSize = glm::vec3(900.0f, 400.0f, 300.0f);
    float m_sigma = 0;
    int m_cgMaxIter = 20;
    float m_cgTol = 1e-3f;
    int m_numPushAppart = 2;
    bool m_accuracyFirst = true;
    
protected:
    std::shared_ptr<FluidInitializer> m_initializer;
    glm::bvec2 m_openBoundsX = glm::bvec2(false);
    glm::bvec2 m_openBoundsY = glm::bvec2(false);
    glm::bvec2 m_openBoundsZ = glm::bvec2(false);
    

public:
    FlipSolverGPUConfig() : IParticleSolverConfig() {}
    ~FlipSolverGPUConfig() override = default;

    std::unique_ptr<ISolverConfig> clone() const override; 

    float getH() const { return 2 * m_partRadius * m_partPerH; }
    float getDensity() const { return m_partPerH * m_partPerH * m_partPerH; }
    
    float getDensityMultiplier() const { return m_densityMultiplier; }
    void setDensityMultiplier(float densityMultiplier) { m_densityMultiplier = std::max(densityMultiplier, 0.0001f); }
    
    float getFlipRatio() const { return m_flipRatio; }
    void setFlipRatio(float flipRatio) { m_flipRatio = glm::clamp(flipRatio, 0.0f, 1.0f); }

    float getSigma() const { return m_sigma; }
    void setSigma(float sigma) { m_sigma = std::max(sigma, 0.0f); }

    int getCgMaxIter() const { return m_cgMaxIter; }
    void setCgMaxIter(int cgMaxIter) { m_cgMaxIter = cgMaxIter; }

    int getNumPushAppart() const { return m_numPushAppart; }
    void setNumPushAppart(int numPushAppart) { m_numPushAppart = numPushAppart; }

    float getCgTol() const { return m_cgTol; }
    void setCgTol(float cgTol) { m_cgTol = cgTol; }

    bool getAccuracyFirst() const { return m_accuracyFirst; }
    void setAccuracyFirst(bool accuracyFirst) { m_accuracyFirst = accuracyFirst; }

    std::shared_ptr<FluidInitializer> getInitializer() const { return m_initializer; }
    virtual void setInitializer(std::shared_ptr<FluidInitializer> initializer) { m_initializer = initializer; }

    virtual void loadOpenGl() { if (m_initializer) m_initializer->loadOpenGl(getH()); }

    float getPartPerH() const { return m_partPerH; }
    void setPartPerH(float partPerH) { m_partPerH = std::max(partPerH, 1.0f); }

    glm::vec3 getDomainSize() const { return m_domainSize; }
    virtual void setDomainSize(glm::vec3 domainSize) { m_domainSize = glm::max(domainSize, 0.f); }


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

    glm::bvec2 getOpenBoundsX() const { return m_openBoundsX; }
    void setOpenBoundsX(glm::bvec2 openBoundsX) { m_openBoundsX = openBoundsX; }

    glm::bvec2 getOpenBoundsY() const { return m_openBoundsY; }
    void setOpenBoundsY(glm::bvec2 openBoundsY) { m_openBoundsY = openBoundsY; }

    glm::bvec2 getOpenBoundsZ() const { return m_openBoundsZ; }
    void setOpenBoundsZ(glm::bvec2 openBoundsZ) { m_openBoundsZ = openBoundsZ; }
};

#endif