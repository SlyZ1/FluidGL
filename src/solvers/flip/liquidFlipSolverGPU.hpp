#ifndef LIQUID_FLIP_SOLVER_GPU_HPP
#define LIQUID_FLIP_SOLVER_GPU_HPP

#include "flipSolverGPU.hpp"
#include "solvers/configs/liquidFlipSolverGPUConfig.hpp"

class LiquidFlipSolverGPU : public FlipSolverGPU {
private:
    LiquidFlipSolverGPUConfig m_config;

    ShaderProgram m_integrateShader = {};

    void deleteBuffers();
    void createBuffers();

    void integrateParticles() override;
    void integrateGrid() override;

public:
    LiquidFlipSolverGPU(LiquidFlipSolverGPUConfig config);
    ~LiquidFlipSolverGPU() override;

    LiquidFlipSolverGPU(const LiquidFlipSolverGPU&) = delete;
    LiquidFlipSolverGPU& operator=(const LiquidFlipSolverGPU&) = delete;

    void accept(ISolverVisitor& visitor) override { visitor.visit(*this); }
    void reload() override;

    const LiquidFlipSolverGPUConfig& getConfig() const override {
        return m_config; 
    }

    LiquidFlipSolverGPUConfig& getDraftConfig() override {
        return static_cast<LiquidFlipSolverGPUConfig&>(ISolver::getDraftConfig());
    }
};

#endif