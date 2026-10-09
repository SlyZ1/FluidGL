#include "triangleRenderer3D.hpp"

using namespace std;
using namespace glm;

string TriangleRenderer::s_shadersPath = "src/shaders/renderers/triangle";

void TriangleRenderer::initOpenGL() {
    glDeleteVertexArrays(1, &m_VAO);
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);
    
    glDeleteBuffers(1, &m_VBO);
    glGenBuffers(1, &m_VBO);
    
    glDeleteBuffers(1, &m_EBO);
    glGenBuffers(1, &m_EBO);
}

TriangleRenderer::TriangleRenderer(std::weak_ptr<Camera> camera) 
: IRenderer(), m_camera(camera) {
    m_triangleShader.create();
    m_triangleShader.load(GL_VERTEX_SHADER, Utils::joinPath(s_shadersPath, "/triangleVert.glsl"));
    m_triangleShader.load(GL_FRAGMENT_SHADER, Utils::joinPath(s_shadersPath, "/triangleFrag.glsl"));
    m_triangleShader.link();

    initOpenGL();
}

TriangleRenderer::~TriangleRenderer() {
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);
    glDeleteBuffers(1, &m_indiciesBuffer);
}

void TriangleRenderer::render() {
    m_triangleShader.use();

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_posBuffer);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec4), (void*)0);
    glVertexAttribDivisor(0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, m_velBuffer);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec4), (void*)0);
    glVertexAttribDivisor(1, 1);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, m_normalBuffer);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vec4), (void*)0);
    glVertexAttribDivisor(2, 0);
    glEnableVertexAttribArray(2);

    if (auto camera = m_camera.lock()){
        glUniformMatrix4fv(ShaderProgram::getVarLoc("uView"), 1, GL_FALSE, &camera->viewMatrix()[0][0]);
        glUniformMatrix4fv(ShaderProgram::getVarLoc("uProj"), 1, GL_FALSE, &camera->projectionMatrix()[0][0]);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDrawElements(GL_TRIANGLES, m_numIndicies, GL_UNSIGNED_INT, 0);
}

void TriangleRenderer::reload() {
    m_triangleShader.reload();
    initOpenGL();
    m_numIndicies = -1;
}

void TriangleRenderer::setIndicies(span<const unsigned int> indicies) {
    m_numIndicies = (int)indicies.size();

    glBindVertexArray(m_VAO);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_numIndicies * sizeof(GLuint), indicies.data(), GL_STATIC_DRAW);
}