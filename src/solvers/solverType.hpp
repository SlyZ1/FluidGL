#ifndef SOLVER_TYPE_HPP
#define SOLVER_TYPE_HPP

#define SOLVER_TYPE_ITER(X) \
    X(EulerGPU, 0) \
    X(LiquidFlipGPU, 1) \
    X(SmokeFlipGPU, 2) \
    X(FlipCPU, 3) \
    X(XpbdCPU, 4) \
    X(MaxType, 5) \

enum SolverType {
#define DECLARE(name, val) name = val,
    SOLVER_TYPE_ITER(DECLARE)
#undef DECLARE
};

#endif