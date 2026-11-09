#ifndef SMOKE_FLIP_SOLVER_GPU_CONFIG_HPP
#define SMOKE_FLIP_SOLVER_GPU_CONFIG_HPP

#include "flipSolverGPUConfig.hpp"

class SmokeFlipSolverGPUConfig : public FlipSolverGPUConfig {
private:
    float m_flotAlpha = 0.6f;
    float m_flotBeta = 4.0f;
    float m_densityDecreaseRate = 0.02f;
    float m_temperatureDecreaseRate = 0.1f;

    FluidInitializer m_densitySource = FluidInitializer();

public:
    SmokeFlipSolverGPUConfig() : FlipSolverGPUConfig() {}
    ~SmokeFlipSolverGPUConfig() override = default;

    void setFluidInitializer(FluidInitializer) override {}
    void setDomainSize(glm::vec3 domainSize) override;

    void setFlotAlpha(float flotAlpha) { m_flotAlpha = flotAlpha; }
    float getFlotAlpha() const { return m_flotAlpha; }

    void setFlotBeta(float flotBeta) { m_flotBeta = flotBeta; }
    float getFlotBeta() const { return m_flotBeta; }

    void setDensityDecreaseRate(float densityDecreaseRate) { m_densityDecreaseRate = densityDecreaseRate; }
    float getDensityDecreaseRate() const { return m_densityDecreaseRate; }

    void setTemperatureDecreaseRate(float temperatureDecreaseRate) { m_temperatureDecreaseRate = temperatureDecreaseRate; }
    float getTemperatureDecreaseRate() const { return m_temperatureDecreaseRate; }

    void setDensitySource(FluidInitializer densitySource) { m_densitySource = densitySource; }
    FluidInitializer getDensitySource() const { return m_densitySource; }
    
    std::unique_ptr<ISolverConfig> clone() const override; 
};

#endif