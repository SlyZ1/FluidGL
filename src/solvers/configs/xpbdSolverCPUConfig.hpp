#ifndef XPBD_SOLVER_CPU_CONFIG_HPP
#define XPBD_SOLVER_CPU_CONFIG_HPP

#include "solverConfig.hpp"

#include <glm/glm.hpp>

class XpbdSolverCPUConfig : public ISolverConfig {
private:
    float m_gravity;
    float m_dt;
    int m_numVertices;

public:
    XpbdSolverCPUConfig() : ISolverConfig() {};
    ~XpbdSolverCPUConfig() override = default;

    std::unique_ptr<ISolverConfig> clone() const override; 

    float getDt() const { return m_dt; }
    void setDt(float dt) { m_dt = dt; }

    float getGravity() const { return m_gravity; }
    void setGravity(float gravity) { m_gravity = gravity; }

    int getNumVertices() const { return m_numVertices; }
    void setNumVertices(int numVertices) { m_numVertices = numVertices; }
};

#endif