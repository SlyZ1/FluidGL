#ifndef TRIANGLE_RENDERER_HPP
#define TRIANGLE_RENDERER_HPP

#include "renderer.hpp"
#include "core/shader_program.hpp"
#include "core/camera.hpp"
#include "solvers/visitors/solverGizmos.hpp"
#include "solvers/solverManager.hpp"
#include <span>

class TriangleRenderer : public IRenderer {
private:
    std::weak_ptr<Camera> m_camera;

    //SolverGizmos m_solverGizmos;
    bool m_drawGizmos = true;

    GLuint m_VBO = 0;
    GLuint m_VAO = 0;
    GLuint m_EBO = 0;

    GLuint m_posBuffer = 0;
    GLuint m_velBuffer = 0;
    GLuint m_normalBuffer = 0;

    GLuint m_indiciesBuffer = 0;
    int m_numIndicies = -1;

    static std::string s_shadersPath;

    ShaderProgram m_triangleShader = {};

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
    TriangleRenderer(std::weak_ptr<Camera> camera);
    ~TriangleRenderer() override;

    TriangleRenderer(const TriangleRenderer&) = delete;
    TriangleRenderer& operator=(const TriangleRenderer&) = delete;

    void render() override;
    void reload() override;

    void setIndicies(std::span<const unsigned int> indices);

    void setPosBuffer(GLuint posBuffer) { m_posBuffer = posBuffer; }
    void setVelBuffer(GLuint velBuffer) { m_velBuffer = velBuffer; }
    void setNormalBuffer(GLuint normalBuffer) { m_normalBuffer = normalBuffer; }
};

#endif