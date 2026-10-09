#ifndef PARTICLE_SOLVER_CONFIG_HPP
#define PARTICLE_SOLVER_CONFIG_HPP

#include "solverConfig.hpp"

class IParticleSolverConfig : public ISolverConfig {
protected:
    int m_partN = 1000;
    float m_partRadius = 1.5f;
    float m_dt = 0.03f;

public:
    IParticleSolverConfig() : ISolverConfig() {}
    IParticleSolverConfig(int partN, float partRadius, float dt) 
    : ISolverConfig(), m_partN(partN), m_partRadius(partRadius), m_dt(dt) {}
    virtual ~IParticleSolverConfig() override = default;

    //virtual std::unique_ptr<ISolverConfig> clone() const override { return std::make_unique<IParticleSolverConfig>(*this); }

    int getPartN() const { return m_partN; }
    void setPartN(int partN) { m_partN = partN; }
    
    float getPartRadius() const { return m_partRadius; }
    void setPartRadius(float partRadius) { m_partRadius = partRadius; }

    float getDt() const { return m_dt; }
    void setDt(float dt) { m_dt = dt; }
};

#endif