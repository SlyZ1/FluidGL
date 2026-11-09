#ifndef EULER_SOLVER_GPU_HPP
#define EULER_SOLVER_GPU_HPP

#include <vector>
#include <glm/glm.hpp>

#include "core/shader_program.hpp"
#include "helpers/metrics.hpp"
#include "helpers/stats.hpp"
#include "cgs/cgs.hpp" 

#include "configs/eulerSolverGPUConfig.hpp"
#include "solver.hpp"

#include "objects/mesh.hpp"

class EulerSolverGPU : public ISolver {
private:
    CGS m_cgs = {};
    EulerSolverGPUConfig m_config;

    // Grid buffers
    GLuint m_velXBuffer = 0;
    GLuint m_velYBuffer = 0;
    GLuint m_velZBuffer = 0;

    // Density
    GLuint m_rhoBuffer = 0;


    GPUTimer m_surfaceTensionTimer = {};
    GPUTimer m_incompressibilityTimer = {};

    StatIndex m_surfaceTensionStatIndex = 0;
    StatIndex m_incompressibilityStatIndex = 0;
    StatIndex m_numCellsStatIndex = 0;

    // Pressure - Incompression
    std::vector<GLuint> m_zBuffers = {};
    std::vector<GLuint> m_rBuffers = {};
    int m_indirectParamsIndex = 0;
    GLuint m_indirectBuffer = 0;
    GLuint m_minusDivBuffer = 0;
    GLuint m_pressureBuffer = 0;

    std::weak_ptr<Mesh> m_obstacle = {};

    void deleteBuffers();
    void createBuffers();
    void initCGS();

    void precomputeCoarseIsAir(bool dispatch = true) {};
    void smooth(int l, int color, glm::ivec3 dim, bool dispatch = true) {};
    void restrict(int l, glm::ivec3 dim, bool dispatch = true) {};
    void prolongate(int l, glm::ivec3 dim, bool dispatch = true) {};
    void MGPrecond(GLuint zBuffer, GLuint rBuffer, bool dispatch = true) {};

    void surfaceTension();
    void solveIncompressibility(int iterations, float tol, bool useCGS = true);

public:
    EulerSolverGPU(EulerSolverGPUConfig config);
    ~EulerSolverGPU() override;

    EulerSolverGPU(const EulerSolverGPU&) = delete;
    EulerSolverGPU& operator=(const EulerSolverGPU&) = delete;

    void accept(ISolverVisitor& visitor) override { visitor.visit(*this); }
    void update() override {};
    void reload() override;

    std::shared_ptr<GLuint> getPosBuffer() const override {return {};};
    std::shared_ptr<GLuint> getVelBuffer() const override {return {};};

    void setObstacle(std::weak_ptr<Mesh> obstacle) { m_obstacle = obstacle; }

    const EulerSolverGPUConfig& getConfig() const override {
        return m_config; 
    }

    EulerSolverGPUConfig& getDraftConfig() override {
        return static_cast<EulerSolverGPUConfig&>(ISolver::getDraftConfig());
    }
};

#endif