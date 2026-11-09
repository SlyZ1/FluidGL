#ifndef SOLVER_VISITOR_HPP
#define SOLVER_VISITOR_HPP

class IParticleSolver;
class EulerSolverGPU;
class FlipSolverCPU;
class FlipSolverGPU;
class LiquidFlipSolverGPU;
class SmokeFlipSolverGPU;
class XpbdSolverCPU;

class ISolverVisitor {
public:
    virtual ~ISolverVisitor() = default;

    virtual void visit(IParticleSolver& solver) = 0;
    virtual void visit(EulerSolverGPU& solver) = 0;
    virtual void visit(FlipSolverCPU& solver) = 0;
    virtual void visit(FlipSolverGPU& solver) = 0;
    virtual void visit(LiquidFlipSolverGPU& solver) = 0;
    virtual void visit(SmokeFlipSolverGPU& solver) = 0;
    virtual void visit(XpbdSolverCPU& solver) = 0;
};

#endif