#ifndef XPBD_SOLVER_HPP
#define XPBD_SOLVER_HPP

#include "solver.hpp"
#include "solvers/configs/xpbdSolverCPUConfig.hpp"

#include <vector>

class XpbdSolverCPU : public ISolver {
private:
    XpbdSolverCPUConfig m_config;

    std::vector<glm::vec3> m_vertPos = {};
    std::vector<glm::vec3> m_vertVel = {};
    std::vector<std::vector<int>> m_neighbours = {};

    GLuint m_posBuffer = 0;
    GLuint m_velBuffer = 0;

    void genBuffers();

    void integrate();
    void applyConstraints();
    void interpolate();

public:
    XpbdSolverCPU(XpbdSolverCPUConfig config);
    ~XpbdSolverCPU() override;

    XpbdSolverCPU(const XpbdSolverCPU&) = delete;
    XpbdSolverCPU& operator=(const XpbdSolverCPU&) = delete;

    void accept(ISolverVisitor& visitor) override { visitor.visit(*this); }
    void update() override;
    void reload() override;
    std::shared_ptr<GLuint> getPosBuffer() const override;
    std::shared_ptr<GLuint> getVelBuffer() const override;

    const XpbdSolverCPUConfig& getConfig() const override {
        return m_config; 
    }

    XpbdSolverCPUConfig& getDraftConfig() override {
        return static_cast<XpbdSolverCPUConfig&>(ISolver::getDraftConfig());
    }
};

#endif