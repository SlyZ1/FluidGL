#ifndef SOLVER_TYPE_HPP
#define SOLVER_TYPE_HPP

#define SOLVER_TYPE_ITER(X) \
    X(FlipGPU, 0) \
    X(FlipCPU, 1) \
    X(XpbdCPU, 2) \
    X(MaxType, 3) \

enum SolverType {
#define DECLARE(name, val) name = val,
    SOLVER_TYPE_ITER(DECLARE)
#undef DECLARE
};

#endif