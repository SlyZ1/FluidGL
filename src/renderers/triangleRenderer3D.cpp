#include "triangleRenderer3D.hpp"

using namespace std;
using namespace glm;

string TriangleRenderer::s_shadersPath = "src/shaders/renderers/triangle";

void TriangleRenderer::initOpenGL() {
    glDeleteVertexArrays(1, &m_VAO);
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);
    
    ShaderProgram::deleteBuffer(m_VBO);
    glGenBuffers(1, &m_VBO);
    
    ShaderProgram::deleteBuffer(m_EBO);
    glGenBuffers(1, &m_EBO);
}

TriangleRenderer::TriangleRenderer(std::weak_ptr<Camera> camera) 
: IRenderer("Triangle Renderer"), m_camera(camera) {
    
    m_renderTimeStatIndex = m_stats->registerTimer("Render Time");
    m_numTriStatIndex = m_stats->registerCounter("Num Triangles");

    m_renderTimer.init();

    m_triangleShader.create();
    m_triangleShader.load(GL_VERTEX_SHADER, Utils::joinPath(s_shadersPath, "/triangleVert.glsl"));
    m_triangleShader.load(GL_FRAGMENT_SHADER, Utils::joinPath(s_shadersPath, "/triangleFrag.glsl"));
    m_triangleShader.link();

    initOpenGL();
}

TriangleRenderer::~TriangleRenderer() {
    glDeleteVertexArrays(1, &m_VAO);
    ShaderProgram::deleteBuffer(m_VBO);
    ShaderProgram::deleteBuffer(m_EBO);
}

void TriangleRenderer::render(bool clear) {
    if (m_numIndicies <= 0 || m_posBuffer == 0) return;
    m_renderTimer.beginFrame();

    m_triangleShader.use();

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_posBuffer);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec4), (void*)0);
    glVertexAttribDivisor(0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, m_normalBuffer);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vec4), (void*)0);
    glVertexAttribDivisor(2, 0);
    glEnableVertexAttribArray(2);

    if (auto camera = m_camera.lock()){
        glUniformMatrix4fv(ShaderProgram::getVarLoc("uView"), 1, GL_FALSE, &camera->viewMatrix()[0][0]);
        glUniformMatrix4fv(ShaderProgram::getVarLoc("uProj"), 1, GL_FALSE, &camera->projectionMatrix()[0][0]);
    }

    glUniformMatrix4fv(ShaderProgram::getVarLoc("uModel"), 1, GL_FALSE, &m_modelMatrix[0][0]);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (clear) glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDrawElements(GL_TRIANGLES, m_numIndicies, GL_UNSIGNED_INT, 0);
    glDisable(GL_CULL_FACE);
    m_renderTimer.endFrame();
    m_stats->setTimer(m_renderTimeStatIndex, m_renderTimer.getLastResultMs());
}

void TriangleRenderer::reload() {
    m_triangleShader.reload();
    initOpenGL();
    m_numIndicies = -1;
}

void TriangleRenderer::setIndicies(span<const unsigned int> indicies) {
    m_numIndicies = (int)indicies.size();
    m_stats->setCounter(m_numTriStatIndex, m_numIndicies / 3);

    glBindVertexArray(m_VAO);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_numIndicies * sizeof(GLuint), indicies.data(), GL_STATIC_DRAW);
}