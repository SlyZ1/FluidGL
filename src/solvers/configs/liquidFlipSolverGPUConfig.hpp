#ifndef LIQUID_FLIP_SOLVER_GPU_CONFIG_HPP
#define LIQUID_FLIP_SOLVER_GPU_CONFIG_HPP

#include "flipSolverGPUConfig.hpp"

class LiquidFlipSolverGPUConfig : public FlipSolverGPUConfig {
private:
    float m_gravity = 9.81f;
public:
    LiquidFlipSolverGPUConfig() : FlipSolverGPUConfig() {}
    ~LiquidFlipSolverGPUConfig() override = default;

    std::unique_ptr<ISolverConfig> clone() const override; 

    float getGravity() const { return m_gravity; }
    void setGravity(float gravity) { m_gravity = std::max(gravity, 0.0f); }
};

#endif