#ifndef SOLVER_UI_HPP
#define SOLVER_UI_HPP

#include "../initializers/initializerVisitor.hpp"
#include "solvers/visitors/solverVisitor.hpp"
#include "solvers/solverManager.hpp"
#include <vector>

class SolverUI : public ISolverVisitor, public IInitializerVisitor {
private:
    void drawInitializer(std::shared_ptr<FluidInitializer> initializer, glm::vec3 dim, float scale);

    glm::vec3 m_maxInitializerSize = glm::vec3(0.0f);
    float m_currentInitializerScale = 1.0f;

public:
    ~SolverUI() override = default;

    void visit(IParticleSolver& solver) override;
    void visit(EulerSolverGPU& solver) override;
    void visit(FlipSolverCPU& solver) override;
    void visit(FlipSolverGPU& solver) override;
    void visit(LiquidFlipSolverGPU& solver) override;
    void visit(SmokeFlipSolverGPU& solver) override;
    void visit(XpbdSolverCPU& solver) override;

    void visit(CubeInitializer& initializer) override;
    void visit(SphereInitializer& initializer) override;

    static std::vector<const char*> solverNames();
    static const char* solverName(SolverType type);
};

#endif