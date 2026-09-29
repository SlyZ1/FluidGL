#ifndef SOLVER_UI_HPP
#define SOLVER_UI_HPP

#include "solvers/solverVisitor.hpp"
#include "solvers/solverManager.hpp"
#include <string>
#include <vector>

class SolverUI : public ISolverVisitor {
public:
    ~SolverUI() override = default;

    void visit(IParticleSolver& solver) override;
    void visit(FlipSolverCPU& solver) override;
    void visit(FlipSolverGPU& solver) override;

    static std::vector<const char*> solverNames();
    static const char* solverName(SolverType type);
};

#endif