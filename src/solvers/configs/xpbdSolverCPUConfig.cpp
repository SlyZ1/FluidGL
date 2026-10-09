#include "xpbdSolverCPUConfig.hpp"

using namespace std;

std::unique_ptr<ISolverConfig> XpbdSolverCPUConfig::clone() const {
    return make_unique<XpbdSolverCPUConfig>(*this);
}