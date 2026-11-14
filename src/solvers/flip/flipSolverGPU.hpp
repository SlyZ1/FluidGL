#ifndef FLIP_SOLVER_GPU_HPP
#define FLIP_SOLVER_GPU_HPP

#include <vector>
#include <glm/glm.hpp>

#include "core/shader_program.hpp"
#include "helpers/metrics.hpp"
#include "helpers/stats.hpp"
#include "cgs/cgs.hpp" 

#include "solvers/configs/flipSolverGPUConfig.hpp"
#include "solvers/particleSolver.hpp"

#include "objects/mesh.hpp"

class FlipSolverGPU : public IParticleSolver {
private:
    CGS m_cgs = {};
    std::shared_ptr<FlipSolverGPUConfig> m_config;

    // FLIP Buffers
    GLuint m_partPosBuffer = 0;
    GLuint m_partVelBuffer = 0;
    GLuint m_oldPartPosBuffer = 0;
    GLuint m_oldPartVelBuffer = 0;

    GLuint m_isDeadBuffer = 0;
    GLuint m_aliveNBuffer = 0;

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
    std::vector<GLuint> m_coarseIsAirBuffers = {};

    // Density
    GLuint m_rhoBuffer = 0;
    GLuint m_smoothRhoBuffer = 0;
    GLuint m_gradRhoBuffer = 0;
    GLuint m_curvatureBuffer = 0;

    // Pressure - Incompression
    std::vector<GLuint> m_zBuffers = {};
    std::vector<GLuint> m_rBuffers = {};
    int m_indirectParamsIndex = 0;
    GLuint m_indirectBuffer = 0;
    GLuint m_minusDivBuffer = 0;
    GLuint m_pressureBuffer = 0;

    // Particle Sort
    GLuint m_cellOfBuffer = 0;
    std::vector<GLuint> m_blockSumBuffers = {};
    GLuint m_cellParticleIdsBuffer = 0;
    GLuint m_firstCellParticleBuffer = 0;
    GLuint m_firstCellParticleBuffer2 = 0;

    ShaderProgram m_integrateGridShader = {};

    ShaderProgram m_collisionShader = {};

    // Particle Sort
    ShaderProgram m_resetUintBuffersShader = {};
    ShaderProgram m_partCountShader = {};
    ShaderProgram m_localSumShader = {};
    ShaderProgram m_smallSumShader = {}; const int SHARED_SIZE = 2048;
    ShaderProgram m_globalSumShader = {};
    ShaderProgram m_cellParticleIdShader = {};
    ShaderProgram m_sortBuffers = {};

    // Push appart
    ShaderProgram m_resetBuffersShader = {};
    ShaderProgram m_getCorrectionsShader = {};
    ShaderProgram m_applyCorrectionsShader = {};

    // A2G
    ShaderProgram m_a2gShader = {};
    ShaderProgram m_applyWieghtsAttributesShader = {};

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
    ShaderProgram m_mgCoarseIsAir = {};
    ShaderProgram m_mgSmooth = {};
    ShaderProgram m_mgRestrict = {};
    ShaderProgram m_mgProlongate = {};
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
    StatIndex m_scanStatIndex = 0;

    StatIndex m_numPartStatIndex = 0;
    StatIndex m_numCellsStatIndex = 0;

    std::weak_ptr<Mesh> m_obstacle = {};
    
    std::vector<std::vector<int>> m_particlesInGrid = {};
 
    
    void deleteBuffers();
    void createBuffers();
    void initCGS();

    void resetFloatBuffer(GLuint buffer, int n);
    void resetUintBuffer(GLuint buffer, int n);
    void prefixSum(GLuint data, int n, int recursionIndex = 0);
    void countingSort();

    void precomputeCoarseIsAir(bool dispatch = true);
    void smooth(int l, int color, glm::ivec3 dim, bool dispatch = true);
    void restrict(int l, glm::ivec3 dim, bool dispatch = true);
    void prolongate(int l, glm::ivec3 dim, bool dispatch = true);
    void MGPrecond(GLuint zBuffer, GLuint rBuffer, bool dispatch = true);

    void pushAppartParticles(int iterations);
    void particleCollisions(std::shared_ptr<Mesh> obstacle);
    void particlesToGrid();
    void surfaceTension();
    void solveIncompressibility(int iterations, float tol, bool useCGS = true);
    void gridToParticles();

protected:
    bool m_isFull = false;
    bool m_collideWithObstacle = true;

    GLuint getIsDeadBuffer() const { return m_isDeadBuffer; }
    GLuint getVelXBuffer() const { return m_velXBuffer; }
    GLuint getVelYBuffer() const { return m_velYBuffer; }
    GLuint getVelZBuffer() const { return m_velZBuffer; }
    GLuint getIsAirBuffer() const { return m_isAirBuffer; }
    GLuint getFirstCellParticleBuffer() const { return m_firstCellParticleBuffer; }

    glm::ivec3 cellToCoord(int cell, int nx, int ny);
    glm::vec3 cellToPos(int cell, int nx, int ny, int nz);
    virtual void integrateParticles() {};
    virtual void integrateGrid() {};
    virtual void sortExtraAttributes(GLuint) {};

public:
    FlipSolverGPU(std::shared_ptr<FlipSolverGPUConfig> config);
    ~FlipSolverGPU() override;

    FlipSolverGPU(const FlipSolverGPU&) = delete;
    FlipSolverGPU& operator=(const FlipSolverGPU&) = delete;

    virtual void accept(ISolverVisitor& visitor) override { visitor.visit(*this); }
    void update() override;
    virtual void reload() override;
    std::shared_ptr<GLuint> getPosBuffer() const override { return std::make_shared<GLuint>(m_partPosBuffer); };
    std::shared_ptr<GLuint> getVelBuffer() const override { return std::make_shared<GLuint>(m_partVelBuffer); };
    std::shared_ptr<GLuint> getPartNBuffer() const { return std::make_shared<GLuint>(m_aliveNBuffer); };

    void attributeToGrid(GLuint attributesBuffer, GLuint gridTex, GLuint weightsBuffer);

    void setObstacle(std::weak_ptr<Mesh> obstacle) { m_obstacle = obstacle; }
    bool getIsFull() const { return m_isFull; }

    const FlipSolverGPUConfig& getConfig() const override {
        return *m_config; 
    }

    FlipSolverGPUConfig& getDraftConfig() override {
        return static_cast<FlipSolverGPUConfig&>(ISolver::getDraftConfig());
    }
};

#endif