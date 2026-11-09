#include "eulerSolverGPUConfig.hpp"

using namespace std;

unique_ptr<ISolverConfig> EulerSolverGPUConfig::clone() const {
    return make_unique<EulerSolverGPUConfig>(*this);
}