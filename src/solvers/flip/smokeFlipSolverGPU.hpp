#ifndef SMOKE_FLIP_SOLVER_GPU_HPP
#define SMOKE_FLIP_SOLVER_GPU_HPP

#include "flipSolverGPU.hpp"
#include "solvers/configs/smokeFlipSolverGPUConfig.hpp"

class SmokeFlipSolverGPU : public FlipSolverGPU {
private:
    SmokeFlipSolverGPUConfig m_config;

    GLuint m_smokeDensityBuffer = 0;
    GLuint m_temperatureBuffer = 0;
    GLuint m_oldDensityBuffer = 0;
    GLuint m_oldTemperatureBuffer = 0;

    GLuint m_gridDensityTexture = 0;
    GLuint m_gridDensityWeightsBuffer = 0;

    GLuint m_vorticityBuffer = 0;
    GLuint m_vorticityForceBuffer = 0;

    unsigned int m_integrationFrame = 0;

    ShaderProgram m_computeVorticityShader = {};
    ShaderProgram m_computeVorticityForceShader = {};

    ShaderProgram m_compensateNumPartShader = {};
    ShaderProgram m_diffuseFieldsShader = {};

    ShaderProgram m_integrateShader = {};
    ShaderProgram m_integrateGridShader = {};
    ShaderProgram m_sortExtraAttributesShader = {};

    int m_allocatedTextureData = 0;
    StatIndex m_allocatedTextureDataStatIndex = 0;

    void deleteBuffers();
    void createBuffers();

    void integrateParticles() override;
    void integrateGrid() override;
    void sortAttribute(GLuint cellParticleIdsBuffer, GLuint attributeBuffer, GLuint sortedAttributeBuffer);
    void sortExtraAttributes(GLuint cellParticleIdsBuffer) override;

public:
    SmokeFlipSolverGPU(SmokeFlipSolverGPUConfig config);
    ~SmokeFlipSolverGPU() override;

    SmokeFlipSolverGPU(const SmokeFlipSolverGPU&) = delete;
    SmokeFlipSolverGPU& operator=(const SmokeFlipSolverGPU&) = delete;

    void accept(ISolverVisitor& visitor) override { visitor.visit(*this); }
    void reload() override;

    std::shared_ptr<GLuint> getDensityBuffer() const { return std::make_shared<GLuint>(m_smokeDensityBuffer); };
    std::shared_ptr<GLuint> getGridDensityTexture() const { return std::make_shared<GLuint>(m_gridDensityTexture); };

    const SmokeFlipSolverGPUConfig& getConfig() const override {
        return m_config; 
    }

    SmokeFlipSolverGPUConfig& getDraftConfig() override {
        return static_cast<SmokeFlipSolverGPUConfig&>(ISolver::getDraftConfig());
    }
};

#endif