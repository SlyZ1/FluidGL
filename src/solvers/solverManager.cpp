#include "solverManager.hpp"

#include "renderers/particleRenderer3D.hpp"

using namespace std;
using namespace glm;

SolverManager::SolverManager(weak_ptr<Camera> camera) 
: m_camera(camera), m_solverGizmos(make_shared<SolverGizmos>(camera)) {}

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
    m_renderCallback = {};
    m_updateCallback = {};

    auto onConfigChanged = [weakSolver   = weak_ptr<FlipSolverCPU>(solver),
                            weakRenderer = weak_ptr<ParticleRenderer3D>(renderer)]() {
        auto s = weakSolver.lock();
        auto r = weakRenderer.lock();
        if (!s || !r) return;

        const FlipSolverCPUConfig& newConfig = s->getConfig();
        r->setNumParticles(newConfig.getPartN());
        r->setPartRadius(newConfig.getPartRadius());
    };

    auto onReload = [weakSolver   = weak_ptr<FlipSolverCPU>(solver),
                            weakRenderer = weak_ptr<ParticleRenderer3D>(renderer)]() {
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

weak_ptr<FlipSolverGPU> SolverManager::instantiate(const FlipSolverGPUConfig& config) {
    shared_ptr<FlipSolverGPU> solver = make_shared<FlipSolverGPU>(config);
    shared_ptr<ParticleRenderer3D> renderer = make_shared<ParticleRenderer3D>(m_camera);
    m_renderCallback = {};
    m_updateCallback = {};

    m_obstacle = make_shared<Mesh>();
    m_obstacle->loadObj("models/suzanne.obj");
    m_obstacle->setPos(vec3(0.0f, -100.0f, -10.0f));
    m_obstacle->setScale(vec3(50.0f));
    solver->setObstacle(m_obstacle);
    
    auto onConfigChanged = [weakSolver   = weak_ptr<FlipSolverGPU>(solver),
                            weakRenderer = weak_ptr<ParticleRenderer3D>(renderer),
                            weakObstacle = weak_ptr<Mesh>(m_obstacle)]() {
        auto s = weakSolver.lock();
        auto r = weakRenderer.lock();
        auto o = weakObstacle.lock();
        if (!s || !r || !o) return;

        const FlipSolverGPUConfig& newConfig = s->getConfig();
        r->setNumParticles(newConfig.getPartN());
        r->setPartRadius(newConfig.getPartRadius());
        
        o->constructSDF(newConfig.getPartRadius());
    };

    auto onReload = [weakSolver   = weak_ptr<FlipSolverGPU>(solver),
                     weakRenderer = weak_ptr<ParticleRenderer3D>(renderer)]() {
        auto s = weakSolver.lock();
        auto r = weakRenderer.lock();
        if (!s || !r) return;
        
        cout << s->getPosBuffer() << " " << *s->getPosBuffer() << " : getPosBuffer" << endl;
        cout << s->getVelBuffer() << " " << *s->getVelBuffer() << " : getVelBuffer" << endl;

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
    m_currentType = SolverType::FlipGPU;
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
    if (m_renderer){
        m_renderer->render(clear);
        if (m_renderCallback) m_renderCallback();
    }

    if (m_drawGizmos){
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glEnable(GL_DEPTH_TEST);
        if (m_solver) m_solver->accept(*m_solverGizmos);
        m_solverGizmos->render();
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