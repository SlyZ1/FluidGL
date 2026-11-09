#include "liquidFlipSolverGPUConfig.hpp"

using namespace std;

unique_ptr<ISolverConfig> LiquidFlipSolverGPUConfig::clone() const {
    return make_unique<LiquidFlipSolverGPUConfig>(*this);
}