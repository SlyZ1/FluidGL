#ifndef SOLVER_MANAGER_HPP
#define SOLVER_MANAGER_HPP

#include "flipSolverCPU.hpp"
#include "flipSolverGPU.hpp"
#include "xpdbSolverCPU.hpp"
#include "solverType.hpp"

#include "renderers/particleRenderer3D.hpp"
#include "solvers/visitors/solverGizmos.hpp"

class SolverManager {
private:
    std::shared_ptr<ISolver> m_solver = {};
    std::weak_ptr<IParticleSolver> m_particleSolver = {};
    std::shared_ptr<IRenderer> m_renderer = {};
    std::weak_ptr<Camera> m_camera = {};
    SolverGizmos m_solverGizmos;
    SolverType m_currentType = SolverType::MaxType;

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

    void update();
    void render();
    
    void reloadSolver();
    void reloadRenderer();
    void togglePause();

    void toggleGizmos(bool drawGizmos) { m_drawGizmos = drawGizmos; if (m_solver) m_solver->accept(m_solverGizmos); }
    bool getGizmosToggled() const { return m_drawGizmos; }

    SolverType getCurrentType() const { return m_currentType; }
};

#endif