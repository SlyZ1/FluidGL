#ifndef SOLVER_INTERFACE_HPP
#define SOLVER_INTERFACE_HPP

#include <memory>
#include <glad/glad.h>

#include "configs/solverConfig.hpp"
#include "helpers/stats.hpp"
#include "solverVisitor.hpp"

class ISolver : public IStatsProvider {
protected:
    std::unique_ptr<ISolverConfig> m_baseConfig;
    std::unique_ptr<ISolverConfig> m_draftConfig;
    bool m_isPaused = true;
public:
    ISolver(std::unique_ptr<ISolverConfig> config, const std::string& statsName) 
    : IStatsProvider(statsName), m_baseConfig(std::move(config)) {};
    virtual ~ISolver() = default;
    virtual void accept(ISolverVisitor& visitor) = 0;
    virtual void update() = 0;
    virtual void reload() = 0;
    virtual GLuint getPosBuffer() const = 0;
    virtual GLuint getVelBuffer() const = 0;

    virtual const ISolverConfig& getConfig() const {
        return *m_baseConfig;
    }

    virtual ISolverConfig& getDraftConfig() {
        if (!m_draftConfig) m_draftConfig = m_baseConfig->clone();
        return *m_draftConfig;
    }

    void applyDraftConfig(){
        if (!m_draftConfig) return;
        m_baseConfig = std::move(m_draftConfig);
        reload();
    }

    bool isPaused() const { return m_isPaused; }
    void setPaused(bool isPaused) { m_isPaused = isPaused; }
};

#endif