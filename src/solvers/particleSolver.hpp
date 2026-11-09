#ifndef PARTICLE_SOLVER_INTERFACE_HPP
#define PARTICLE_SOLVER_INTERFACE_HPP

#include "solver.hpp"
#include "configs/particleSolverConfig.hpp"

class IParticleSolver : public ISolver {
public:
    IParticleSolver(std::shared_ptr<IParticleSolverConfig> config, const std::string& statsName) : ISolver(std::move(config), statsName) {};
    virtual ~IParticleSolver() = default;

    virtual const IParticleSolverConfig& getConfig() const override {
        return static_cast<const IParticleSolverConfig&>(*m_baseConfig); 
    }
};

#endif