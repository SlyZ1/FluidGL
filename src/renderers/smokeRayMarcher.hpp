#ifndef SMOKE_RAY_MARCHER_HPP
#define SMOKE_RAY_MARCHER_HPP

#include "renderer.hpp"
#include "core/camera.hpp"
#include "core/app.hpp"
#include "core/shader_program.hpp"

class SmokeRayMarcher : public IRenderer {
private:
    std::weak_ptr<Camera> m_camera;
    std::weak_ptr<App> m_app;

    GLuint m_VBO = 0;
    GLuint m_VAO = 0;
    GLuint m_EBO = 0;

    GLuint m_smokeGridDensityTexture = 0; 

    ShaderProgram m_smokeRayMarchShader = {};

    StatIndex m_renderTimeStatIndex = 0;
    GPUTimer m_renderTimer = {};

    float m_absorption = 0.3f;
    float m_lightAbsorption = 0.15f;
    float m_numStepSizeRatio = 2.f;
    float m_numLightStepSizeRatio = 1.0f;
    float m_shadowMultiplier = 0.7f;
    glm::vec3 m_lightDir = glm::normalize(glm::vec3(1, 2, 0));

    glm::vec3 m_domainSize = glm::vec3(0.0f);
    float m_h = 0.0f;

    void initOpenGL();

public:
    SmokeRayMarcher(std::weak_ptr<App> app, std::weak_ptr<Camera> camera);
    ~SmokeRayMarcher() override;

    SmokeRayMarcher(const SmokeRayMarcher&) = delete;
    SmokeRayMarcher& operator=(const SmokeRayMarcher&) = delete;

    void setSmokeGridDensityTexture(GLuint smokeGridDensityTexture) { m_smokeGridDensityTexture = smokeGridDensityTexture; }
    void setDomainSize(glm::vec3 domainSize) { m_domainSize = domainSize; }
    void setH(float h) { m_h = h; }

    void setAbsorption(float absorption) { m_absorption = absorption; }
    float getAbsorption() { return m_absorption; }
    void setLightAbsorption(float lightAbsorption) { m_lightAbsorption = lightAbsorption; }
    float getLightAbsorption() { return m_lightAbsorption; }

    void setNumStepSizeRatio(float numStepSizeRatio) { m_numStepSizeRatio = numStepSizeRatio; }
    float getNumStepSizeRatio() { return m_numStepSizeRatio; }
    void setNumLightStepSizeRatio(float numLightStepSizeRatio) { m_numLightStepSizeRatio = numLightStepSizeRatio; }
    float getNumLightStepSizeRatio() { return m_numLightStepSizeRatio; }

    void setShadowMultiplier(float shadowMultiplier) { m_shadowMultiplier = shadowMultiplier; }
    float getShadowMultiplier() { return m_shadowMultiplier; }

    void setLightDir(glm::vec3 lightDir) { m_lightDir = normalize(lightDir); }
    glm::vec3 getLightDir() { return m_lightDir; }

    void render(bool clear = false) override;
    void reload() override;

    void accept(IRendererVisitor& visitor) override { visitor.visit(*this); };
};

#endif