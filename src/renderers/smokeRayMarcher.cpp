#include "smokeRayMarcher.hpp"

using namespace std;
using namespace glm;

void SmokeRayMarcher::initOpenGL(){
    glDeleteVertexArrays(1, &m_VAO);
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);
    
    ShaderProgram::deleteBuffer(m_VBO);
    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, s_cubeVerts.size() * sizeof(float), s_cubeVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    
    ShaderProgram::deleteBuffer(m_EBO);
    glGenBuffers(1, &m_EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, s_cubeIndices.size() * sizeof(GLuint), s_cubeIndices.data(), GL_STATIC_DRAW);
}

SmokeRayMarcher::SmokeRayMarcher(weak_ptr<Camera> camera) : IRenderer("Smoke Ray Marcher"), m_camera(camera) {
    m_renderTimeStatIndex = m_stats->registerTimer("Render Time");

    m_renderTimer.init();

    m_smokeRayMarchShader.create();
    m_smokeRayMarchShader.load(GL_VERTEX_SHADER, "src/shaders/renderers/raymarching/smokeraymarchVert.glsl");
    m_smokeRayMarchShader.load(GL_FRAGMENT_SHADER, "src/shaders/renderers/raymarching/smokeraymarchFrag.glsl");
    m_smokeRayMarchShader.link();

    initOpenGL();
}

SmokeRayMarcher::~SmokeRayMarcher() {
    glDeleteVertexArrays(1, &m_VAO);
    ShaderProgram::deleteBuffer(m_VBO);
    ShaderProgram::deleteBuffer(m_EBO);
}

void SmokeRayMarcher::render(bool clear){
    m_renderTimer.beginFrame();
    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glBindVertexArray(m_VAO);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (clear) glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_smokeRayMarchShader.use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, m_smokeGridDensityTexture);
    glUniform1i(ShaderProgram::getVarLoc("densityTexture"), 0);

    glUniform3f(ShaderProgram::getVarLoc("domainSize"), m_domainSize.x, m_domainSize.y, m_domainSize.z);
    glUniform1f(ShaderProgram::getVarLoc("h"), m_h);

    glUniform1f(ShaderProgram::getVarLoc("absorption"), m_absorption);
    glUniform1f(ShaderProgram::getVarLoc("numStepSizeRatio"), m_numStepSizeRatio);
    glUniform1ui(ShaderProgram::getVarLoc("smoothDensity"), m_smoothDensity);

    if (auto camera = m_camera.lock()){
        glUniformMatrix4fv(ShaderProgram::getVarLoc("uView"), 1, GL_FALSE, &camera->viewMatrix()[0][0]);
        glUniformMatrix4fv(ShaderProgram::getVarLoc("uProj"), 1, GL_FALSE, &camera->projectionMatrix()[0][0]);
        glUniform3f(ShaderProgram::getVarLoc("cameraPosition"), camera->getPosition().x, camera->getPosition().y, camera->getPosition().z);
    }

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    m_renderTimer.endFrame();
    m_stats->setTimer(m_renderTimeStatIndex, m_renderTimer.getLastResultMs());
}

void SmokeRayMarcher::reload(){
    m_smokeRayMarchShader.reload();
    //initOpenGL();
}
