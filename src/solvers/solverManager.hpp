#ifndef SOLVER_MANAGER_HPP
#define SOLVER_MANAGER_HPP

#include "eulerSolverGPU.hpp"
#include "flip/flipSolverCPU.hpp"
#include "flip/liquidFlipSolverGPU.hpp"
#include "flip/smokeFlipSolverGPU.hpp"
#include "xpdbSolverCPU.hpp"
#include "solverType.hpp"

#include "renderers/renderer.hpp"
#include "solvers/visitors/solverGizmos.hpp"
#include "objects/mesh.hpp"

class SolverManager {
private:
    std::shared_ptr<ISolver> m_solver = {};
    std::weak_ptr<IParticleSolver> m_particleSolver = {};
    std::shared_ptr<IRenderer> m_renderer = {};
    std::weak_ptr<Camera> m_camera = {};
    std::shared_ptr<SolverGizmos> m_solverGizmos = {};
    SolverType m_currentType = SolverType::MaxType;

    std::shared_ptr<Mesh> m_obstacle = {};

    std::function<void()> m_updateCallback = {};
    std::function<void()> m_renderCallback = {};

public:
    SolverManager(std::weak_ptr<Camera> camera);
    ~SolverManager();

    SolverManager(const SolverManager&) = delete;
    SolverManager& operator=(const SolverManager&) = delete;

    std::weak_ptr<ISolver> instantiate(SolverType type);
    std::weak_ptr<EulerSolverGPU> instantiate(const EulerSolverGPUConfig& config);
    std::weak_ptr<FlipSolverCPU> instantiate(const FlipSolverCPUConfig& config);
    std::weak_ptr<LiquidFlipSolverGPU> instantiate(const LiquidFlipSolverGPUConfig& config);
    std::weak_ptr<SmokeFlipSolverGPU> instantiate(const SmokeFlipSolverGPUConfig& config);
    std::weak_ptr<XpbdSolverCPU> instantiate(const XpbdSolverCPUConfig& config);

    std::weak_ptr<ISolver> getSolver() const { return m_solver; }
    std::weak_ptr<IParticleSolver> getParticleSolver() const { return m_particleSolver; }

    std::weak_ptr<IRenderer> getRenderer() const { return m_renderer; }
    std::weak_ptr<SolverGizmos> getSolverGizmos() const { return m_solverGizmos; }
    std::weak_ptr<Mesh> getObstacle() const { return m_obstacle; }
    void setObstacle(std::shared_ptr<Mesh> obstacle) { m_obstacle = obstacle; }

    void update();
    void render(bool clear = false);
    
    void reloadSolver();
    void reloadRenderer();
    void togglePause();

    SolverType getCurrentType() const { return m_currentType; }
};

#endif