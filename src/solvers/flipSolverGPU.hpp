#ifndef FLIP_SOLVER_GPU_HPP
#define FLIP_SOLVER_GPU_HPP

#include <vector>
#include <glm/glm.hpp>

#include "core/shader_program.hpp"
#include "helpers/metrics.hpp"
#include "helpers/stats.hpp"
#include "cgs/cgs.hpp" 

#include "configs/flipSolverGPUConfig.hpp"
#include "particleSolver.hpp"

class FlipSolverGPU : public IParticleSolver {
private:
    CGS m_cgs = {};
    FlipSolverGPUConfig m_config;

    static std::string s_shadersPath;

    // FLIP Buffers
    GLuint m_partPosBuffer = 0;
    GLuint m_oldPartPosBuffer = 0;
    GLuint m_partVelBuffer = 0;
    GLuint m_velXBuffer = 0;
    GLuint m_velYBuffer = 0;
    GLuint m_velZBuffer = 0;
    GLuint m_oldVelXBuffer = 0;
    GLuint m_oldVelYBuffer = 0;
    GLuint m_oldVelZBuffer = 0;
    GLuint m_rXBuffer = 0;
    GLuint m_rYBuffer = 0;
    GLuint m_rZBuffer = 0;
    GLuint m_isAirBuffer = 0;

    // Density
    GLuint m_rhoBuffer = 0;
    GLuint m_smoothRhoBuffer = 0;
    GLuint m_gradRhoBuffer = 0;
    GLuint m_curvatureBuffer = 0;

    // Pressure - Incompression
    GLuint m_minusDivBuffer = 0;
    GLuint m_pressureBuffer = 0;

    // Particle Sort
    GLuint m_cellOfBuffer = 0;
    GLuint m_blockSumBuffer = 0;
    GLuint m_cellParticleIdsBuffer = 0;
    GLuint m_firstCellParticleBuffer = 0;
    GLuint m_firstCellParticleBuffer2 = 0;

    ShaderProgram m_integrateShader = {};
    ShaderProgram m_integrateGridShader = {};

    ShaderProgram m_collisionShader = {};

    // Particle Sort
    ShaderProgram m_resetUintBuffersShader = {};
    ShaderProgram m_partCountShader = {};
    ShaderProgram m_localSumShader = {};
    ShaderProgram m_smallSumShader = {};
    ShaderProgram m_globalSumShader = {};
    ShaderProgram m_cellParticleIdShader = {};

    // Push appart
    ShaderProgram m_resetBuffersShader = {};
    ShaderProgram m_getCorrectionsShader = {};
    ShaderProgram m_applyCorrectionsShader = {};

    // P2G
    ShaderProgram m_resetFloatBufferShader = {};
    ShaderProgram m_p2gShader = {};
    ShaderProgram m_applyWeightsShader = {};

    // Density
    ShaderProgram m_computeRhoShader = {};
    ShaderProgram m_smoothDataShader = {};
    ShaderProgram m_computeGradShader = {};
    ShaderProgram m_computeCurvatureShader = {};
    
    // Pressure - Incompression
    ShaderProgram m_solveIncompressibilityShader = {};
    ShaderProgram m_sparseMatVecShader = {};
    ShaderProgram m_computeMinusDivShader = {};
    ShaderProgram m_pressureToVelShader = {};
    ShaderProgram m_setAirCellsToZeroShader = {};
    
    ShaderProgram m_g2pShader = {};

    GPUTimer m_integrateTimer = {};
    GPUTimer m_pushAppartTimer = {};
    GPUTimer m_collisionTimer = {};
    GPUTimer m_p2gTimer = {};
    GPUTimer m_surfaceTensionTimer = {};
    GPUTimer m_incompressibilityTimer = {};
    GPUTimer m_g2pTimer = {};
    GPUTimer m_scanTimer = {};
    StatIndex m_integrationStatIndex = 0;
    StatIndex m_pushAppartStatIndex = 0;
    StatIndex m_collisionStatIndex = 0;
    StatIndex m_p2gStatIndex = 0;
    StatIndex m_surfaceTensionStatIndex = 0;
    StatIndex m_incompressibilityStatIndex = 0;
    StatIndex m_g2pStatIndex = 0;

    StatIndex m_numPartStatIndex = 0;
    StatIndex m_numCellsStatIndex = 0;

    glm::vec2 m_obstaclePos = glm::vec2(0.f);
    glm::vec2 m_obstacleVel = glm::vec2(0.f);
    float m_obstacleRadius = 0.f;
    
    std::vector<std::vector<int>> m_particlesInGrid = {};

    glm::ivec3 cellToCoord(int cell, int nx, int ny);
    glm::vec3 cellToPos(int cell, int nx, int ny, int nz);

    static void loadCompute(ShaderProgram& prog, const std::string& relativePath);
 
    void deleteBuffers();
    void createBuffers();

    void resetFloatBuffer(GLuint buffer, int n);
    void resetUintBuffer(GLuint buffer, int n);
    void prefixSum(GLuint data, GLuint blockSum, int n);
    void countingSort();

    void integrateParticles();
    void pushAppartParticles(int iterations);
    void particleCollisions();
    void particlesToGrid();
    void surfaceTension();
    void solveIncompressibility(int iterations, float tol, bool useCGS = true);
    void gridToParticles();

public:
    FlipSolverGPU(FlipSolverGPUConfig config);
    ~FlipSolverGPU() override;

    FlipSolverGPU(const FlipSolverGPU&) = delete;
    FlipSolverGPU& operator=(const FlipSolverGPU&) = delete;

    void accept(ISolverVisitor& visitor) override { visitor.visit(*this); }
    void update() override;
    void reload() override;
    GLuint getPosBuffer() const override { return m_partPosBuffer; };
    GLuint getVelBuffer() const override { return m_partVelBuffer; };

    void updateObstacle(glm::vec2 pos, glm::vec2 vel, float radius);

    const FlipSolverGPUConfig& getConfig() const override {
        return m_config; 
    }

    FlipSolverGPUConfig& getDraftConfig() override {
        return static_cast<FlipSolverGPUConfig&>(ISolver::getDraftConfig());
    }

    bool is3D() const override { return true; }
};

#endif