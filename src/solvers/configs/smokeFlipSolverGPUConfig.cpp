#include "smokeFlipSolverGPUConfig.hpp"

#include "solvers/initializers/cubeInitializer.hpp"

using namespace std;
using namespace glm;

SmokeFlipSolverGPUConfig::SmokeFlipSolverGPUConfig() : FlipSolverGPUConfig() {
    m_initializer = std::make_shared<CubeInitializer>(glm::vec3(0.0f), glm::vec3(0.0f), 1.0f); 
}

unique_ptr<ISolverConfig> SmokeFlipSolverGPUConfig::clone() const {
    return make_unique<SmokeFlipSolverGPUConfig>(*this);
}

void SmokeFlipSolverGPUConfig::setDomainSize(vec3 domainSize) {
    FlipSolverGPUConfig::setDomainSize(domainSize);
    static_pointer_cast<CubeInitializer>(m_initializer)->setSizeAndPos(domainSize, vec3(0.0f), getPartRadius());
}

void SmokeFlipSolverGPUConfig::loadOpenGl() {
    if (m_densitySource) m_densitySource->loadOpenGl(getH(), getPartRadius());
    FlipSolverGPUConfig::loadOpenGl();
}