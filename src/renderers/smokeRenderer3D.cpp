#include "smokeRenderer3D.hpp"

#include "helpers/utils.hpp"

using namespace std;
using namespace glm;

string SmokeRenderer3D::s_shadersPath = "src/shaders/renderers/particle";

SmokeRenderer3D::SmokeRenderer3D(weak_ptr<Camera> camera) 
: IRenderer("Particle Renderer"), m_camera(camera) {

    m_renderTimeStatIndex = m_stats->registerTimer("Render Time");

    m_numPartStatIndex = m_stats->registerCounter("Num Particles");
    m_numTriStatIndex = m_stats->registerCounter("Num Triangles");

    m_renderTimer.init();

    m_particleShader.create();
    m_particleShader.load(GL_VERTEX_SHADER, Utils::joinPath(s_shadersPath, "/smokeParticleVert.glsl"));
    m_particleShader.load(GL_FRAGMENT_SHADER, Utils::joinPath(s_shadersPath, "/smokeParticleFrag.glsl"));
    m_particleShader.link();

    initOpenGL();
}

SmokeRenderer3D::~SmokeRenderer3D(){
    glDeleteVertexArrays(1, &m_VAO);
    ShaderProgram::deleteBuffer(m_VBO);
    ShaderProgram::deleteBuffer(m_EBO);
    ShaderProgram::deleteBuffer(m_drawBuffer);
}

void SmokeRenderer3D::initOpenGL(){
    glDeleteVertexArrays(1, &m_VAO);
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);
    
    ShaderProgram::deleteBuffer(m_VBO);
    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, s_quadVerts.size() * sizeof(float), s_quadVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    
    ShaderProgram::deleteBuffer(m_EBO);
    glGenBuffers(1, &m_EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, s_quadIndices.size() * sizeof(GLuint), s_quadIndices.data(), GL_STATIC_DRAW);

    ShaderProgram::deleteBuffer(m_drawBuffer);
    glGenBuffers(1, &m_drawBuffer);
    DrawElementsIndirectCommand command = { 6, 0, 0, 0, 0 };
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_drawBuffer);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(DrawElementsIndirectCommand), &command, GL_DYNAMIC_DRAW);
}

void SmokeRenderer3D::renderLogic(bool clear) {
    if (!m_posBuffer || !m_velBuffer) return;
    m_particleShader.use();

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, *m_posBuffer);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec4), (void*)0);
    glVertexAttribDivisor(1, 1);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, *m_velBuffer);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vec4), (void*)0);
    glVertexAttribDivisor(2, 1);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, *m_densityBuffer);

    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
    glVertexAttribDivisor(3, 1);
    glEnableVertexAttribArray(3);

    glUniform1f(ShaderProgram::getVarLoc("particleRadius"), m_partRadius);
    if (auto camera = m_camera.lock()){
        glUniformMatrix4fv(ShaderProgram::getVarLoc("uView"), 1, GL_FALSE, &camera->viewMatrix()[0][0]);
        glUniformMatrix4fv(ShaderProgram::getVarLoc("uProj"), 1, GL_FALSE, &camera->projectionMatrix()[0][0]);
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (clear) glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void SmokeRenderer3D::render(bool clear){
    m_renderTimer.beginFrame();

    renderLogic(clear);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    if (!m_renderIndirect){
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, m_numParticles);
        m_stats->setCounter(m_numPartStatIndex, m_numParticles);
        m_stats->setCounter(m_numTriStatIndex, m_numParticles * 2);
    }
    else{
        glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

        glBindBuffer(GL_COPY_READ_BUFFER, *m_partNBuffer);
        glBindBuffer(GL_COPY_WRITE_BUFFER, m_drawBuffer);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, sizeof(GLuint), sizeof(GLuint));

        glMemoryBarrier(GL_COMMAND_BARRIER_BIT);

        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_drawBuffer);
        glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (void*)0);

        if (m_internalFrameCounter % 50 == 0){
            ShaderProgram::indirectBarrier();
            int drawnParticles = 0;
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, *m_partNBuffer);
            glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), &drawnParticles);
            m_stats->setCounter(m_numPartStatIndex, drawnParticles);
            m_stats->setCounter(m_numTriStatIndex, drawnParticles * 2);
        }
    }

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);

    m_renderTimer.endFrame();

    m_stats->setTimer(m_renderTimeStatIndex, m_renderTimer.getLastResultMs());
    m_internalFrameCounter++;
}

void SmokeRenderer3D::reload(){
    m_particleShader.reload();

    initOpenGL();
}