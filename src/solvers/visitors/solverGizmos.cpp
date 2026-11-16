#include "solverGizmos.hpp"

#include "solvers/flip/flipSolverCPU.hpp"
#include "solvers/flip/liquidFlipSolverGPU.hpp"
#include "solvers/flip/smokeFlipSolverGPU.hpp"
#include "solvers/xpdbSolverCPU.hpp"

#include "solvers/initializers/cubeInitializer.hpp"
#include "solvers/initializers/sphereInitializer.hpp"

using namespace std;
using namespace glm;

#define UPDATE_WIREFRAME(shape, dataContainer, changed, newPointer, arg1, arg2, color) \
    if (changed){ \
        dataContainer.data = newPointer; \
        if (dataContainer.index < 0) dataContainer.index = m_wireframes->add##shape(arg1, arg2, color); \
        else m_wireframes->update##shape(dataContainer.index, arg1, arg2, color); \
        m_wireframes->uploadData(); \
    }

SolverGizmos::SolverGizmos(weak_ptr<Camera> camera) : IStatsProvider("Solver's gizmos") {
    m_wireframes = make_unique<Wireframes>(camera);

    m_renderStatIndex = m_stats->registerTimer("Render Time");
    m_renderTimer.init();
}

SolverGizmos::~SolverGizmos() {}

void SolverGizmos::resetWireframes() {
    m_wireframes->reset();
    m_solverDomainData = {};
    m_solverInitializerData = {};
    m_smokeDensitySourceData = {};
}

void SolverGizmos::drawInitializerWireframes(shared_ptr<FluidInitializer> initializer, WireframeData<InitializerBounds>& initializerData, float partRadius){
    m_currentInitializerData = &initializerData;
    m_currentPartRadius = partRadius;
    initializer->accept(*this);
}

void SolverGizmos::visit(IParticleSolver&) {}

void SolverGizmos::visit(EulerSolverGPU&){
    
}

void SolverGizmos::visit(FlipSolverCPU& solver) {
    m_renderTimer.beginFrame();
    const FlipSolverCPUConfig& config = solver.getDraftConfig();

    if (m_solverType != SolverType::FlipCPU) resetWireframes();
    m_solverType = SolverType::FlipCPU;
    
    vec3 newDomainSize = vec3(config.getDomainSize(), 0);
    vec3 domainBounds = m_solverDomainData.data * 0.5f;
    UPDATE_WIREFRAME(
        Box2D, 
        m_solverDomainData,
        m_solverDomainData.data != newDomainSize,
        newDomainSize, 
        -domainBounds, 
        domainBounds, 
        m_domainColor
    )
    m_renderTimer.endFrame();
    m_stats->setTimer(m_renderStatIndex, m_renderTimer.getLastResultMs());
}

void SolverGizmos::visit(FlipSolverGPU& solver) {
    const FlipSolverGPUConfig& config = solver.getDraftConfig();
    
    vec3 newDomainSize = config.getDomainSize();
    vec3 domainBounds = newDomainSize * 0.5f;
    UPDATE_WIREFRAME(
        Box, 
        m_solverDomainData, 
        m_solverDomainData.data != newDomainSize,
        newDomainSize, 
        -domainBounds, 
        domainBounds, 
        m_domainColor
    )

    std::shared_ptr<FluidInitializer> initializerBounds = config.getInitializer();
    drawInitializerWireframes(initializerBounds, m_solverInitializerData, config.getPartRadius());
}

void SolverGizmos::visit(LiquidFlipSolverGPU& solver) {
    m_renderTimer.beginFrame();
    if (m_solverType != SolverType::LiquidFlipGPU) resetWireframes();
    m_solverType = SolverType::LiquidFlipGPU;

    visit((FlipSolverGPU&)solver);
    m_renderTimer.endFrame();
    m_stats->setTimer(m_renderStatIndex, m_renderTimer.getLastResultMs());
}

void SolverGizmos::visit(SmokeFlipSolverGPU& solver) {
    m_renderTimer.beginFrame();
    if (m_solverType != SolverType::SmokeFlipGPU) resetWireframes();
    m_solverType = SolverType::SmokeFlipGPU;

    const SmokeFlipSolverGPUConfig& config = solver.getDraftConfig();

    std::shared_ptr<FluidInitializer> densitySource = config.getDensitySource();
    drawInitializerWireframes(densitySource, m_smokeDensitySourceData, config.getPartRadius());

    visit((FlipSolverGPU&)solver);
    m_renderTimer.endFrame();
    m_stats->setTimer(m_renderStatIndex, m_renderTimer.getLastResultMs());
}

void SolverGizmos::visit(XpbdSolverCPU&) {
    m_renderTimer.beginFrame();
    //const XpbdSolverCPUConfig& config = solver.getDraftConfig();

    if (m_solverType != SolverType::XpbdCPU) resetWireframes();
    m_solverType = SolverType::XpbdCPU;

    m_renderTimer.endFrame();
    m_stats->setTimer(m_renderStatIndex, m_renderTimer.getLastResultMs());
}


void SolverGizmos::visit(CubeInitializer& initializer) {
    UPDATE_WIREFRAME(
        Box, 
        (*m_currentInitializerData),
        initializer.hasChanged(),
        InitializerBounds(initializer),
        initializer.min(m_currentPartRadius), 
        initializer.max(m_currentPartRadius), 
        m_initializerColor
    )
}

void SolverGizmos::visit(SphereInitializer& initializer) {
    UPDATE_WIREFRAME(
        Sphere,
        (*m_currentInitializerData),
        initializer.hasChanged(),
        InitializerBounds(initializer),
        initializer.pos(m_currentPartRadius), 
        initializer.getRadius(m_currentPartRadius),
        m_initializerColor
    )
}



void SolverGizmos::render() const {
    m_wireframes->render();
}