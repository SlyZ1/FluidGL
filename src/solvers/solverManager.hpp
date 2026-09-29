#ifndef SOLVER_MANAGER_HPP
#define SOLVER_MANAGER_HPP

#include "flipSolverCPU.hpp"
#include "flipSolverGPU.hpp"

#define SOLVER_TYPE_ITER(X) \
    X(FlipGPU, 0) \
    X(FlipCPU, 1) \
    X(MaxType, 2) \

enum SolverType {
#define DECLARE(name, val) name = val,
    SOLVER_TYPE_ITER(DECLARE)
#undef DECLARE
};

class SolverManager {
private:
    std::shared_ptr<ISolver> m_solver;
    std::weak_ptr<IParticleSolver> m_particleSolver;
    SolverType m_currentType = SolverType::MaxType;
public:
    SolverManager();
    ~SolverManager();

    SolverManager(const SolverManager&) = delete;
    SolverManager& operator=(const SolverManager&) = delete;

    std::weak_ptr<ISolver> instantiate(SolverType type);
    std::weak_ptr<FlipSolverCPU> instantiate(const FlipSolverCPUConfig& config);
    std::weak_ptr<FlipSolverGPU> instantiate(const FlipSolverGPUConfig& config);

    std::weak_ptr<ISolver> getSolver() const { return m_solver; }
    std::weak_ptr<IParticleSolver> getParticleSolver() const { return m_particleSolver; }

    SolverType getCurrentType() const { return m_currentType; }
};

#endif