#ifndef GPU_RENDERER_3D_HPP
#define GPU_RENDERER_3D_HPP

#include "renderer.hpp"
#include "core/shader_program.hpp"
#include "core/camera.hpp"

class ParticleRenderer3D : public IRenderer {
private:
    std::weak_ptr<Camera> m_camera;

    GLuint m_VBO = 0;
    GLuint m_VAO = 0;
    GLuint m_EBO = 0;

    GLuint m_posBuffer = 0;
    GLuint m_velBuffer = 0;

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

    ShaderProgram m_particleShader = {};

    static constexpr std::array<float, 12> s_quadVerts = {
        1.f,  1.f, 0.f,
        1.f, -1.f, 0.f,
        -1.f, -1.f, 0.f,
        -1.f,  1.f, 0.f,
    };
    static constexpr std::array<unsigned int, 6> s_quadIndices = {
        0, 1, 2,
        0, 2, 3
    };

    void initOpenGL();
     
public:
    ParticleRenderer3D(std::weak_ptr<Camera> camera);
    ~ParticleRenderer3D() override;

    ParticleRenderer3D(const ParticleRenderer3D&) = delete;
    ParticleRenderer3D& operator=(const ParticleRenderer3D&) = delete;

    void setPosBuffer(GLuint posBuffer) { m_posBuffer = posBuffer; }
    void setVelBuffer(GLuint velBuffer) { m_velBuffer = velBuffer; }
    void setPartRadius(float partRadius) { m_partRadius = partRadius; }
    void setNumParticles(int numParticles) { m_numParticles = numParticles; }

    void render() override;
    void reload() override;
};

#endif 