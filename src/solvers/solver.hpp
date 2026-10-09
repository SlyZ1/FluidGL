#ifndef SOLVER_INTERFACE_HPP
#define SOLVER_INTERFACE_HPP

#include <memory>
#include <glad/glad.h>
#include <functional>

#include "configs/solverConfig.hpp"
#include "helpers/stats.hpp"
#include "solvers/visitors/solverVisitor.hpp"

class ISolver : public IStatsProvider {
protected:
    std::unique_ptr<ISolverConfig> m_baseConfig;
    std::unique_ptr<ISolverConfig> m_draftConfig;
    std::function<void()> m_configChangedCallback = {}; 
    std::function<void()> m_reloadCallback = {}; 
    bool m_isPaused = true;
public:
    ISolver(std::unique_ptr<ISolverConfig> config, const std::string& statsName) 
    : IStatsProvider(statsName), m_baseConfig(std::move(config)) {};
    virtual ~ISolver() = default;
    virtual void accept(ISolverVisitor& visitor) = 0;
    virtual void update() = 0;
    virtual void reload() { m_reloadCallback(); };
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
        m_configChangedCallback();
    }

    void setConfigChangedCallback(std::function<void()> configChangedCallback) { m_configChangedCallback = configChangedCallback; }
    void setReloadCallback(std::function<void()> reloadCallback) { m_reloadCallback = reloadCallback; }

    bool isPaused() const { return m_isPaused; }
    void setPaused(bool isPaused) { m_isPaused = isPaused; }
};

#endif