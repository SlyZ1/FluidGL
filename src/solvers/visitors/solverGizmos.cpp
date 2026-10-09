#include "solverGizmos.hpp"

#include "solvers/flipSolverCPU.hpp"
#include "solvers/flipSolverGPU.hpp"
#include "solvers/xpdbSolverCPU.hpp"

using namespace std;
using namespace glm;

#define UPDATE_WIREFRAME(shape, name, newVal, min, max, color) \
    if (newVal != name.data){ \
        name.data = newVal; \
        if (name.index < 0) name.index = m_wireframes->add##shape(min, max, color); \
        else m_wireframes->update##shape(name.index, min, max, color); \
        m_wireframes->uploadData(); \
    }

SolverGizmos::SolverGizmos(weak_ptr<Camera> camera) {
    m_wireframes = make_unique<Wireframes>(camera);
}

SolverGizmos::~SolverGizmos() {}

void SolverGizmos::resetWireframes() {
    m_wireframes->reset();
    m_solverDomainData = {};
    m_solverInitializerData = {};
}

void SolverGizmos::visit(IParticleSolver&) {}

void SolverGizmos::visit(FlipSolverCPU& solver) {
    const FlipSolverCPUConfig& config = solver.getDraftConfig();

    if (m_solverType != SolverType::FlipCPU) resetWireframes();
    m_solverType = SolverType::FlipCPU;
    
    vec3 newDomainSize = vec3(config.getDomainSize(), 0);
    vec3 domainBounds = m_solverDomainData.data * 0.5f;
    UPDATE_WIREFRAME(Box2D, m_solverDomainData, newDomainSize, -domainBounds, domainBounds, m_domainColor)
}

void SolverGizmos::visit(FlipSolverGPU& solver) {
    const FlipSolverGPUConfig& config = solver.getDraftConfig();

    if (m_solverType != SolverType::FlipGPU) resetWireframes();
    m_solverType = SolverType::FlipGPU;
    
    vec3 newDomainSize = config.getDomainSize();
    vec3 domainBounds = config.getDomainSize() * 0.5f;
    UPDATE_WIREFRAME(Box, m_solverDomainData, newDomainSize, -domainBounds, domainBounds, m_domainColor)

    FluidInitializer initializerBounds = config.getFluidInitializer();
    UPDATE_WIREFRAME(Box, m_solverInitializerData, initializerBounds, initializerBounds.min, initializerBounds.max, m_initializerColor)
}

void SolverGizmos::visit(XpbdSolverCPU& solver) {
    const XpbdSolverCPUConfig& config = solver.getDraftConfig();

    if (m_solverType != SolverType::XpbdCPU) resetWireframes();
    m_solverType = SolverType::XpbdCPU;
}

void SolverGizmos::render() const {
    m_wireframes->render();
}