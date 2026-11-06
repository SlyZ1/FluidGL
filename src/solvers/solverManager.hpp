#ifndef SOLVER_MANAGER_HPP
#define SOLVER_MANAGER_HPP

#include "flipSolverCPU.hpp"
#include "flipSolverGPU.hpp"
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

    bool m_drawGizmos = true;

public:
    SolverManager(std::weak_ptr<Camera> camera);
    ~SolverManager();

    SolverManager(const SolverManager&) = delete;
    SolverManager& operator=(const SolverManager&) = delete;

    std::weak_ptr<ISolver> instantiate(SolverType type);
    std::weak_ptr<FlipSolverCPU> instantiate(const FlipSolverCPUConfig& config);
    std::weak_ptr<FlipSolverGPU> instantiate(const FlipSolverGPUConfig& config);
    std::weak_ptr<XpbdSolverCPU> instantiate(const XpbdSolverCPUConfig& config);

    std::weak_ptr<ISolver> getSolver() const { return m_solver; }
    std::weak_ptr<IParticleSolver> getParticleSolver() const { return m_particleSolver; }

    std::weak_ptr<IRenderer> getRenderer() const { return m_renderer; }
    std::weak_ptr<SolverGizmos> getSolverGizmos() const { return m_solverGizmos; }
    std::weak_ptr<Mesh> getObstacle() const { return m_obstacle; }

    void update();
    void render(bool clear = false);
    
    void reloadSolver();
    void reloadRenderer();
    void togglePause();

    void toggleGizmos(bool drawGizmos) { m_drawGizmos = drawGizmos; if (m_solver) m_solver->accept(*m_solverGizmos); }
    bool getGizmosToggled() const { return m_drawGizmos; }


    SolverType getCurrentType() const { return m_currentType; }
};

#endif