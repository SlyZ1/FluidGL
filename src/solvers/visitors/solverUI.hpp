#ifndef SOLVER_UI_HPP
#define SOLVER_UI_HPP

#include "solvers/visitors/solverVisitor.hpp"
#include "solvers/solverManager.hpp"
#include <string>
#include <vector>

class SolverUI : public ISolverVisitor {
private:
    FluidInitializer drawFluidInitializer(FluidInitializer initializer, glm::vec3 dim, float scale);

public:
    ~SolverUI() override = default;

    void visit(IParticleSolver& solver) override;
    void visit(EulerSolverGPU& solver) override;
    void visit(FlipSolverCPU& solver) override;
    void visit(FlipSolverGPU& solver) override;
    void visit(LiquidFlipSolverGPU& solver) override;
    void visit(SmokeFlipSolverGPU& solver) override;
    void visit(XpbdSolverCPU& solver) override;

    static std::vector<const char*> solverNames();
    static const char* solverName(SolverType type);
};

#endif