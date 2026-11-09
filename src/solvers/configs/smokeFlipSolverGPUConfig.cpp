#include "smokeFlipSolverGPUConfig.hpp"

using namespace std;
using namespace glm;

unique_ptr<ISolverConfig> SmokeFlipSolverGPUConfig::clone() const {
    return make_unique<SmokeFlipSolverGPUConfig>(*this);
}

void SmokeFlipSolverGPUConfig::setDomainSize(vec3 domainSize) {
    FlipSolverGPUConfig::setDomainSize(domainSize);
    m_fluidInitializer.setSizeAndPos(domainSize - vec3(0.0f), vec3(0.0f), getPartRadius());
}