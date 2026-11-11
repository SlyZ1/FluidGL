#ifndef LIQUID_PARTICLE_RENDERER_3D_HPP
#define LIQUID_PARTICLE_RENDERER_3D_HPP

#include "renderer.hpp"
#include "core/shader_program.hpp"
#include "core/camera.hpp"

class LiquidRenderer3D : public IRenderer {
private:
    std::weak_ptr<Camera> m_camera;

    GLuint m_VBO = 0;
    GLuint m_VAO = 0;
    GLuint m_EBO = 0;
    GLuint m_drawBuffer = 0;

    struct DrawElementsIndirectCommand {
        GLuint count;
        GLuint instanceCount;
        GLuint firstIndex;
        GLint  baseVertex;
        GLuint baseInstance;
    };

    std::shared_ptr<GLuint> m_partNBuffer = 0;
    bool m_renderIndirect = false;

    std::shared_ptr<GLuint> m_posBuffer = 0;
    std::shared_ptr<GLuint> m_velBuffer = 0;

    float m_partRadius = 0;
    int m_numParticles = 0;

    static std::string s_shadersPath;

    // GLuint gridVBO = 0;
    // GLuint posesVBO = 0;
    // GLuint colorsVBO = 0;
    // GLuint depthFBO = 0;

    // GLuint cumulativeDepthFBO = 0;
    // GLuint normalFBO = 0;
    // GLuint blurredFBO = 0;

    // GLuint depthTex = 0;
    // GLuint depthColorTex = 0;
    // GLuint cumulativeDepthTex = 0;
    // GLuint normalTex = 0;
    // GLuint blurredTex = 0;

    // ShaderProgram cumulativeParticleShader = {};
    // ShaderProgram blurShader = {};
    // ShaderProgram normalShader = {};
    // ShaderProgram gridShader = {};
    // ShaderProgram waterShader = {};

    StatIndex m_renderTimeStatIndex = 0;

    StatIndex m_numPartStatIndex = 0;
    StatIndex m_numTriStatIndex = 0;

    GPUTimer m_renderTimer = {};

    ShaderProgram m_particleShader = {};

    unsigned int m_internalFrameCounter = 0;

    void initOpenGL();
    void renderLogic(bool clear);

public:
    LiquidRenderer3D(std::weak_ptr<Camera> camera);
    ~LiquidRenderer3D() override;

    LiquidRenderer3D(const LiquidRenderer3D&) = delete;
    LiquidRenderer3D& operator=(const LiquidRenderer3D&) = delete;

    void setPosBuffer(std::shared_ptr<GLuint> posBuffer) { m_posBuffer = posBuffer; }
    void setVelBuffer(std::shared_ptr<GLuint> velBuffer) { m_velBuffer = velBuffer; }
    void setPartRadius(float partRadius) { m_partRadius = partRadius; }
    void setNumParticles(int numParticles) { m_numParticles = numParticles; }
    void setNumParticlesBuffer(std::shared_ptr<GLuint> partNBuffer) { m_partNBuffer = partNBuffer; }
    void setRenderMode(bool renderIndirect) { m_renderIndirect = renderIndirect; }

    void render(bool clear = false) override;
    void reload() override;
};

#endif 