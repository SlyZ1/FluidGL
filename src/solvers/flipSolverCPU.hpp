#ifndef FLIP_SOLVER_CPU_HPP
#define FLIP_SOLVER_CPU_HPP

#include <vector>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

#include "configs/flipSolverCPUConfig.hpp"
#include "particleSolver.hpp"

class FlipSolverCPU : public IParticleSolver {
private:
    FlipSolverCPUConfig m_config = {};

    GLuint m_posVBO = 0;
    GLuint m_velVBO = 0;
    
    glm::vec2 m_obstaclePos = glm::vec2(0.f);
    glm::vec2 m_obstacleVel = glm::vec2(0.f);
    float m_obstacleRadius = 0.f;
    
    std::vector<glm::vec4> m_partPos = {};
    std::vector<glm::vec4> m_partVel = {};
    std::vector<float> m_oldVelX = {};
    std::vector<float> m_oldVelY = {};
    std::vector<float> m_velX = {};
    std::vector<float> m_velY = {};
    std::vector<float> m_rX = {};
    std::vector<float> m_rY = {};
    std::vector<bool> m_isAir = {};
    std::vector<bool> m_isWall = {};
    std::vector<std::vector<int>> m_particlesInGrid = {};

    void genBuffers();

    glm::ivec2 cellToCoord(int cell, int nx);
    int coordToCell(glm::ivec2 coord, int nx, int ny);
    int posToCell(glm::vec2 pos, int nx, int ny);
    glm::vec2 cellToPos(int cell, int nx, int ny);
    glm::vec2 coordToPos(glm::ivec2 coord, int nx, int ny);

    void posToCoordAndDp(glm::vec2 pos, int nx, int ny, glm::ivec2& coord, glm::vec2& dp);

    void integrateParticles();
    void pushAppartParticles();
    void particleCollisions();
    void particlesToGrid();
    void solveIncompressibility(int iterations);
    void gridToParticles();

public:
    FlipSolverCPU(FlipSolverCPUConfig config);
    ~FlipSolverCPU() override {};

    FlipSolverCPU(const FlipSolverCPU&) = delete;
    FlipSolverCPU& operator=(const FlipSolverCPU&) = delete;

    void accept(ISolverVisitor& visitor) override { visitor.visit(*this); }
    void update() override;
    void reload() override;
    std::shared_ptr<GLuint> getPosBuffer() const override;
    std::shared_ptr<GLuint> getVelBuffer() const override;

    const std::vector<glm::vec4>& getPos() { return m_partPos; };
    const std::vector<glm::vec4>& getVel() { return m_partVel; };
    std::vector<glm::vec4> getGrid(float width);
    std::vector<glm::vec4> getCells();
    std::vector<glm::vec4> getCellColors();
    void updateObstacle(glm::vec2 pos, glm::vec2 vel, float radius);

    const FlipSolverCPUConfig& getConfig() const override {
        return m_config; 
    }

    FlipSolverCPUConfig& getDraftConfig() override {
        return static_cast<FlipSolverCPUConfig&>(ISolver::getDraftConfig());
    }
};

#endif