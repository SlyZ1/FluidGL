#include "solverManager.hpp"

using namespace std;

SolverManager::SolverManager(weak_ptr<Camera> camera) 
: m_camera(camera), m_solverGizmos(camera) {}

SolverManager::~SolverManager() {}

weak_ptr<ISolver> SolverManager::instantiate(SolverType type) {
    switch (type)
    {
    case SolverType::FlipGPU:
        return instantiate(FlipSolverGPUConfig());
    
    case SolverType::FlipCPU:
        return instantiate(FlipSolverCPUConfig());
    
    case SolverType::XpbdCPU:
        return instantiate(XpbdSolverCPUConfig());

    default:
        return instantiate(FlipSolverGPUConfig());
    }
}

weak_ptr<FlipSolverCPU> SolverManager::instantiate(const FlipSolverCPUConfig& config) {
    shared_ptr<FlipSolverCPU> solver = make_shared<FlipSolverCPU>(config);
    shared_ptr<ParticleRenderer3D> renderer = make_shared<ParticleRenderer3D>(m_camera);

    auto updateRendererData = [weakSolver   = weak_ptr<FlipSolverCPU>(solver),
                            weakRenderer = weak_ptr<ParticleRenderer3D>(renderer)]() {
        auto s = weakSolver.lock();
        auto r = weakRenderer.lock();
        if (!s || !r) return;

        const FlipSolverCPUConfig& newConfig = s->getConfig();
        r->setNumParticles(newConfig.getPartN());
        r->setPartRadius(newConfig.getPartRadius());
    };

    auto updateRendererBuffers = [weakSolver   = weak_ptr<FlipSolverCPU>(solver),
                            weakRenderer = weak_ptr<ParticleRenderer3D>(renderer)]() {
        auto s = weakSolver.lock();
        auto r = weakRenderer.lock();
        if (!s || !r) return;
        
        r->setPosBuffer(s->getPosBuffer());
        r->setVelBuffer(s->getVelBuffer());
    };

    solver->setConfigChangedCallback(updateRendererData);
    solver->setReloadCallback(updateRendererBuffers);
    updateRendererData();
    updateRendererBuffers();

    m_renderer = renderer;

    m_solver = solver;
    m_particleSolver = solver;
    m_currentType = SolverType::FlipCPU;
    return solver;
}

weak_ptr<FlipSolverGPU> SolverManager::instantiate(const FlipSolverGPUConfig& config) {
    shared_ptr<FlipSolverGPU> solver = make_shared<FlipSolverGPU>(config);
    shared_ptr<ParticleRenderer3D> renderer = make_shared<ParticleRenderer3D>(m_camera);
    
    auto updateRendererData = [weakSolver   = weak_ptr<FlipSolverGPU>(solver),
                            weakRenderer = weak_ptr<ParticleRenderer3D>(renderer)]() {
        auto s = weakSolver.lock();
        auto r = weakRenderer.lock();
        if (!s || !r) return;

        const FlipSolverGPUConfig& newConfig = s->getConfig();
        r->setNumParticles(newConfig.getPartN());
        r->setPartRadius(newConfig.getPartRadius());
    };

    auto updateRendererBuffers = [weakSolver   = weak_ptr<FlipSolverGPU>(solver),
                            weakRenderer = weak_ptr<ParticleRenderer3D>(renderer)]() {
        auto s = weakSolver.lock();
        auto r = weakRenderer.lock();
        if (!s || !r) return;
        
        r->setPosBuffer(s->getPosBuffer());
        r->setVelBuffer(s->getVelBuffer());
    };

    solver->setConfigChangedCallback(updateRendererData);
    solver->setReloadCallback(updateRendererBuffers);
    updateRendererData();
    updateRendererBuffers();

    m_renderer = renderer;

    m_solver = solver;
    m_particleSolver = solver;
    m_currentType = SolverType::FlipGPU;
    return solver;
}

weak_ptr<XpbdSolverCPU> SolverManager::instantiate(const XpbdSolverCPUConfig& config) {
    shared_ptr<XpbdSolverCPU> solver = make_shared<XpbdSolverCPU>(config);
    m_solver = solver;
    m_currentType = SolverType::XpbdCPU;
    return solver;
}

void SolverManager::update(){
    if (m_solver) m_solver->update();
}

void SolverManager::render(){
    if (m_renderer) m_renderer->render();

    if (m_drawGizmos){
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glEnable(GL_DEPTH_TEST);
        if (m_solver) m_solver->accept(m_solverGizmos);
        m_solverGizmos.render();
    }
}

void SolverManager::reloadSolver() {
    if (m_solver) m_solver->reload();
}

void SolverManager::reloadRenderer() {
    if (m_renderer) m_renderer->reload();
}

void SolverManager::togglePause() {
    if (m_solver) m_solver->setPaused(!m_solver->isPaused());
}