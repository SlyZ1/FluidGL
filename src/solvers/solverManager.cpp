#include "solverManager.hpp"

#include "renderers/liquidRenderer3D.hpp"
#include "renderers/smokeRenderer3D.hpp"

using namespace std;
using namespace glm;

SolverManager::SolverManager(weak_ptr<Camera> camera) 
: m_camera(camera), m_solverGizmos(make_shared<SolverGizmos>(camera)) {}

SolverManager::~SolverManager() {}

weak_ptr<ISolver> SolverManager::instantiate(SolverType type) {
    switch (type)
    {
    case SolverType::EulerGPU:
        return instantiate(EulerSolverGPUConfig());

    case SolverType::LiquidFlipGPU:
        return instantiate(LiquidFlipSolverGPUConfig());
    
    case SolverType::FlipCPU:
        return instantiate(FlipSolverCPUConfig());
    
    case SolverType::XpbdCPU:
        return instantiate(XpbdSolverCPUConfig());

    default:
        return instantiate(LiquidFlipSolverGPUConfig());
    }
}

weak_ptr<EulerSolverGPU> SolverManager::instantiate(const EulerSolverGPUConfig& config){
    shared_ptr<EulerSolverGPU> solver = make_shared<EulerSolverGPU>(config);
    m_renderCallback = {};
    m_updateCallback = {};
    m_solver = solver;
    m_currentType = SolverType::EulerGPU;
    return solver;
}

weak_ptr<FlipSolverCPU> SolverManager::instantiate(const FlipSolverCPUConfig& config) {
    shared_ptr<FlipSolverCPU> solver = make_shared<FlipSolverCPU>(config);
    shared_ptr<LiquidRenderer3D> renderer = make_shared<LiquidRenderer3D>(m_camera);
    m_renderCallback = {};
    m_updateCallback = {};

    auto onConfigChanged = [weakSolver   = weak_ptr<FlipSolverCPU>(solver),
                            weakRenderer = weak_ptr<LiquidRenderer3D>(renderer)]() {
        auto s = weakSolver.lock();
        auto r = weakRenderer.lock();
        if (!s || !r) return;

        const FlipSolverCPUConfig& newConfig = s->getConfig();
        r->setNumParticles(newConfig.getPartN());
        r->setPartRadius(newConfig.getPartRadius());
    };

    auto onReload = [weakSolver   = weak_ptr<FlipSolverCPU>(solver),
                            weakRenderer = weak_ptr<LiquidRenderer3D>(renderer)]() {
        auto s = weakSolver.lock();
        auto r = weakRenderer.lock();
        if (!s || !r) return;
        
        r->setPosBuffer(s->getPosBuffer());
        r->setVelBuffer(s->getVelBuffer());
    };

    solver->setConfigChangedCallback(onConfigChanged);
    solver->setReloadCallback(onReload);
    onConfigChanged();
    onReload();

    m_renderer = renderer;

    m_solver = solver;
    m_particleSolver = solver;
    m_currentType = SolverType::FlipCPU;
    return solver;
}

weak_ptr<LiquidFlipSolverGPU> SolverManager::instantiate(const LiquidFlipSolverGPUConfig& config) {
    shared_ptr<LiquidFlipSolverGPU> solver = make_shared<LiquidFlipSolverGPU>(config);
    shared_ptr<LiquidRenderer3D> renderer = make_shared<LiquidRenderer3D>(m_camera);
    m_renderCallback = {};
    m_updateCallback = {};

    if (m_obstacle) solver->setObstacle(m_obstacle);
    
    auto onConfigChanged = [weakSolver   = weak_ptr<LiquidFlipSolverGPU>(solver),
                            weakRenderer = weak_ptr<LiquidRenderer3D>(renderer),
                            weakObstacle = weak_ptr<Mesh>(m_obstacle)]() {
        auto s = weakSolver.lock();
        auto r = weakRenderer.lock();
        auto o = weakObstacle.lock();
        if (!s || !r) return;

        const LiquidFlipSolverGPUConfig& newConfig = s->getConfig();
        r->setNumParticles(newConfig.getPartN());
        r->setPartRadius(newConfig.getPartRadius());
        
        if (o) o->constructSDF(newConfig.getPartRadius());
    };

    auto onReload = [weakSolver   = weak_ptr<LiquidFlipSolverGPU>(solver),
                     weakRenderer = weak_ptr<LiquidRenderer3D>(renderer)]() {
        auto s = weakSolver.lock();
        auto r = weakRenderer.lock();
        if (!s || !r) return;

        r->setPosBuffer(s->getPosBuffer());
        r->setVelBuffer(s->getVelBuffer());
        r->setNumParticlesBuffer(s->getPartNBuffer());
        r->setRenderMode(true);
    };

    solver->setConfigChangedCallback(onConfigChanged);
    solver->setReloadCallback(onReload);
    onConfigChanged();
    onReload();

    m_renderer = renderer;

    m_solver = solver;
    m_particleSolver = solver;
    m_currentType = SolverType::LiquidFlipGPU;
    return solver;
}

weak_ptr<SmokeFlipSolverGPU> SolverManager::instantiate(const SmokeFlipSolverGPUConfig& config) {
    shared_ptr<SmokeFlipSolverGPU> solver = make_shared<SmokeFlipSolverGPU>(config);
    shared_ptr<SmokeRenderer3D> renderer = make_shared<SmokeRenderer3D>(m_camera);
    m_renderCallback = {};
    m_updateCallback = {};

    if (m_obstacle) solver->setObstacle(m_obstacle);
    
    auto onConfigChanged = [weakSolver   = weak_ptr<SmokeFlipSolverGPU>(solver),
                            weakRenderer = weak_ptr<SmokeRenderer3D>(renderer),
                            weakObstacle = weak_ptr<Mesh>(m_obstacle)]() {
        auto s = weakSolver.lock();
        auto r = weakRenderer.lock();
        auto o = weakObstacle.lock();
        if (!s || !r) return;

        const SmokeFlipSolverGPUConfig& newConfig = s->getConfig();
        r->setNumParticles(newConfig.getPartN());
        r->setPartRadius(newConfig.getPartRadius());
        
        if (o) o->constructSDF(newConfig.getPartRadius());
    };

    auto onReload = [weakSolver   = weak_ptr<SmokeFlipSolverGPU>(solver),
                     weakRenderer = weak_ptr<SmokeRenderer3D>(renderer)]() {
        auto s = weakSolver.lock();
        auto r = weakRenderer.lock();
        if (!s || !r) return;

        r->setPosBuffer(s->getPosBuffer());
        r->setVelBuffer(s->getVelBuffer());
        r->setDensityBuffer(s->getDensityBuffer());
        r->setNumParticlesBuffer(s->getPartNBuffer());
        r->setRenderMode(true);
    };

    solver->setConfigChangedCallback(onConfigChanged);
    solver->setReloadCallback(onReload);
    onConfigChanged();
    onReload();

    m_renderer = renderer;

    m_solver = solver;
    m_particleSolver = solver;
    m_currentType = SolverType::SmokeFlipGPU;
    return solver;
}

weak_ptr<XpbdSolverCPU> SolverManager::instantiate(const XpbdSolverCPUConfig& config) {
    shared_ptr<XpbdSolverCPU> solver = make_shared<XpbdSolverCPU>(config);
    m_renderCallback = {};
    m_updateCallback = {};
    m_solver = solver;
    m_currentType = SolverType::XpbdCPU;
    return solver;
}

void SolverManager::update(){
    if (m_solver){
        m_solver->update();
        if (m_updateCallback) m_updateCallback();
    }
}

void SolverManager::render(bool clear){
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_DEPTH_TEST);
    if (m_solver) m_solver->accept(*m_solverGizmos);
    m_solverGizmos->render();

    if (m_renderer){
        m_renderer->render(clear);
        if (m_renderCallback) m_renderCallback();
    }
}

void SolverManager::reloadSolver() {
    if (m_solver){
        m_solver->reload();
    } 
}

void SolverManager::reloadRenderer() {
    if (m_renderer){
        m_renderer->reload();
    }
}

void SolverManager::togglePause() {
    if (m_solver) m_solver->setPaused(!m_solver->isPaused());
}