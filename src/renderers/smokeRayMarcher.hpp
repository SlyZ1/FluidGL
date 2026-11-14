#ifndef SMOKE_RAY_MARCHER_HPP
#define SMOKE_RAY_MARCHER_HPP

#include "renderer.hpp"
#include "core/camera.hpp"
#include "core/shader_program.hpp"

class SmokeRayMarcher : public IRenderer {
private:
    std::weak_ptr<Camera> m_camera;

    GLuint m_VBO = 0;
    GLuint m_VAO = 0;
    GLuint m_EBO = 0;

    GLuint m_smokeGridDensityTexture = 0; 

    ShaderProgram m_smokeRayMarchShader = {};

    StatIndex m_renderTimeStatIndex = 0;
    GPUTimer m_renderTimer = {};

    float m_absorption = 0.1f;
    float m_numStepSizeRatio = 10.f;
    uint m_smoothDensity = 1u;

    glm::vec3 m_domainSize = glm::vec3(0.0f);
    float m_h = 0.0f;

    void initOpenGL();

public:
    SmokeRayMarcher(std::weak_ptr<Camera> camera);
    ~SmokeRayMarcher() override;

    SmokeRayMarcher(const SmokeRayMarcher&) = delete;
    SmokeRayMarcher& operator=(const SmokeRayMarcher&) = delete;

    void setSmokeGridDensityTexture(GLuint smokeGridDensityTexture) { m_smokeGridDensityTexture = smokeGridDensityTexture; }
    void setDomainSize(glm::vec3 domainSize) { m_domainSize = domainSize; }
    void setH(float h) { m_h = h; }

    void setAbsorption(float absorption) { m_absorption = absorption; }
    float getAbsorption() { return m_absorption; }
    void setNumStepSizeRatio(float numStepSizeRatio) { m_numStepSizeRatio = numStepSizeRatio; }
    float getNumStepSizeRatio() { return m_numStepSizeRatio; }
    void setSmoothDensity(uint smoothDensity) { m_smoothDensity = smoothDensity; }
    uint getSmoothDensity() { return m_smoothDensity; }

    void render(bool clear = false) override;
    void reload() override;
};

#endif