#include "solverManager.hpp"

using namespace std;

SolverManager::SolverManager() {}

SolverManager::~SolverManager() {}

weak_ptr<ISolver> SolverManager::instantiate(SolverType type) {
    switch (type)
    {
    case SolverType::FlipGPU:
        return instantiate(FlipSolverGPUConfig());
    
    case SolverType::FlipCPU:
        return instantiate(FlipSolverCPUConfig());

    default:
        return instantiate(FlipSolverGPUConfig());
    }
}

weak_ptr<FlipSolverCPU> SolverManager::instantiate(const FlipSolverCPUConfig& config) {
    shared_ptr<FlipSolverCPU> solver = make_shared<FlipSolverCPU>(config);
    m_solver = solver;
    m_particleSolver = solver;
    m_currentType = SolverType::FlipCPU;
    return solver;
}

weak_ptr<FlipSolverGPU> SolverManager::instantiate(const FlipSolverGPUConfig& config) {
    shared_ptr<FlipSolverGPU> solver = make_shared<FlipSolverGPU>(config);
    m_solver = solver;
    m_particleSolver = solver;
    m_currentType = SolverType::FlipGPU;
    return solver;
}