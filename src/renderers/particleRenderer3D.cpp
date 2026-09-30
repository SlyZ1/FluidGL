#include "particleRenderer3D.hpp"

#include "helpers/utils.hpp"

using namespace std;
using namespace glm;

string ParticleRenderer3D::s_shadersPath = "src/shaders/renderers/particle";

ParticleRenderer3D::ParticleRenderer3D(const SolverManager& solverManager, weak_ptr<Camera> camera) 
: IRenderer(), m_solverManager(solverManager), m_camera(camera), m_solverGizmos(camera) {

    m_particleShader.create();
    m_particleShader.load(GL_VERTEX_SHADER, Utils::joinPath(s_shadersPath, "/particleVert.glsl"));
    m_particleShader.load(GL_FRAGMENT_SHADER, Utils::joinPath(s_shadersPath, "/particleFrag.glsl"));
    m_particleShader.link();

    // cumulativeParticleShader.create();
    // cumulativeParticleShader.load(GL_VERTEX_SHADER, "src/shaders/particleVert.glsl");
    // cumulativeParticleShader.load(GL_FRAGMENT_SHADER, "src/shaders/particleFragCumulative.glsl");
    // cumulativeParticleShader.link();

    // gridShader.create();
    // gridShader.load(GL_VERTEX_SHADER, "src/shaders/gridVert.glsl");
    // gridShader.load(GL_FRAGMENT_SHADER, "src/shaders/gridFrag.glsl");
    // gridShader.link();

    // blurShader.create();
    // blurShader.load(GL_VERTEX_SHADER, "src/shaders/blurVert.glsl");
    // blurShader.load(GL_FRAGMENT_SHADER, "src/shaders/blurFrag.glsl");
    // blurShader.link();

    // normalShader.create();
    // normalShader.load(GL_VERTEX_SHADER, "src/shaders/normalVert.glsl");
    // normalShader.load(GL_FRAGMENT_SHADER, "src/shaders/normalFrag.glsl");
    // normalShader.link();

    // waterShader.create();
    // waterShader.load(GL_VERTEX_SHADER, "src/shaders/waterVert.glsl");
    // waterShader.load(GL_FRAGMENT_SHADER, "src/shaders/waterFrag.glsl");
    // waterShader.link();

    initOpenGL();
}

ParticleRenderer3D::~ParticleRenderer3D(){
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);

    // glDeleteBuffers(1, &gridVBO);
    // glDeleteBuffers(1, &posesVBO);
    // glDeleteBuffers(1, &colorsVBO);
    
    // glDeleteFramebuffers(1, &depthFBO);
    // glDeleteFramebuffers(1, &cumulativeDepthFBO);
    // glDeleteFramebuffers(1, &normalFBO);
    // glDeleteFramebuffers(1, &blurredFBO);

    // glDeleteTextures(1, &depthTex);
    // glDeleteTextures(1, &depthColorTex);
    // glDeleteTextures(1, &cumulativeDepthTex);
    // glDeleteTextures(1, &normalTex);
    // glDeleteTextures(1, &blurredTex);

    // cumulativeParticleShader.destroy();
    // blurShader.destroy();
    // normalShader.destroy();
    // gridShader.destroy();
    // waterShader.destroy();
}

void ParticleRenderer3D::initOpenGL(){
    glDeleteVertexArrays(1, &m_VAO);
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);
    
    glDeleteBuffers(1, &m_VBO);
    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, s_quadVerts.size() * sizeof(float), s_quadVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    
    glDeleteBuffers(1, &m_EBO);
    glGenBuffers(1, &m_EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, s_quadIndices.size() * sizeof(GLuint), s_quadIndices.data(), GL_STATIC_DRAW);

    // glGenTextures(1, &depthTex);
    // glBindTexture(GL_TEXTURE_2D, depthTex);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, app->width(), app->height(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glGenTextures(1, &depthColorTex);
    // glBindTexture(GL_TEXTURE_2D, depthColorTex);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, app->width(), app->height(), 0, GL_RGBA, GL_FLOAT, nullptr); 
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glGenTextures(1, &cumulativeDepthTex);
    // glBindTexture(GL_TEXTURE_2D, cumulativeDepthTex);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, app->width(), app->height(), 0, GL_RGBA, GL_FLOAT, nullptr); 
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glGenTextures(1, &normalTex);
    // glBindTexture(GL_TEXTURE_2D, normalTex);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, app->width(), app->height(), 0, GL_RGBA, GL_FLOAT, nullptr); 
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glGenTextures(1, &blurredTex);
    // glBindTexture(GL_TEXTURE_2D, blurredTex);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, app->width(), app->height(), 0, GL_RGBA, GL_FLOAT, nullptr); 
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // glGenFramebuffers(1, &depthFBO);
    // glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, depthColorTex, 0);
    // glGenFramebuffers(1, &cumulativeDepthFBO);
    // glBindFramebuffer(GL_FRAMEBUFFER, cumulativeDepthFBO);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cumulativeDepthTex, 0);
    // glGenFramebuffers(1, &normalFBO);
    // glBindFramebuffer(GL_FRAMEBUFFER, normalFBO);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, normalTex, 0);
    // glGenFramebuffers(1, &blurredFBO);
    // glBindFramebuffer(GL_FRAMEBUFFER, blurredFBO);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurredTex, 0);
}

void ParticleRenderer3D::render(){
    auto solver = m_solverManager.getParticleSolver().lock();
    if (!solver) return;

    m_particleShader.use();

    const IParticleSolverConfig& config = solver->getConfig(); 

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, solver->getPosBuffer());

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec4), (void*)0);
    glVertexAttribDivisor(1, 1);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, solver->getVelBuffer());

    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vec4), (void*)0);
    glVertexAttribDivisor(3, 1);
    glEnableVertexAttribArray(3);

    glUniform1f(ShaderProgram::getVarLoc("particleRadius"), config.getPartRadius());
    if (auto camera = m_camera.lock()){
        glUniformMatrix4fv(ShaderProgram::getVarLoc("uView"), 1, GL_FALSE, &camera->viewMatrix()[0][0]);
        glUniformMatrix4fv(ShaderProgram::getVarLoc("uProj"), 1, GL_FALSE, &camera->projectionMatrix()[0][0]);
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, config.getPartN());

    // cumulativeParticleShader.use();

    // glUniform1f(ShaderProgram::getVarLoc("particleRadius"), particleRadius);
    // glUniformMatrix4fv(ShaderProgram::getVarLoc("uView"), 1, GL_FALSE, &camera->viewMatrix()[0][0]);
    // glUniformMatrix4fv(ShaderProgram::getVarLoc("uProj"), 1, GL_FALSE, &uProj[0][0]);
    
    // glBindFramebuffer(GL_FRAMEBUFFER, cumulativeDepthFBO);
    // glClear(GL_COLOR_BUFFER_BIT);
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_ONE, GL_ONE);
    // glDisable(GL_DEPTH_TEST);
    // glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, numParticle);

    // normalShader.use();
    
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, depthColorTex);
    // glUniform1i(ShaderProgram::getVarLoc("depthTex"), 0);
    
    // glUniform2f(ShaderProgram::getVarLoc("viewport"), app->width(), app->height());
    // glUniformMatrix4fv(ShaderProgram::getVarLoc("uInvView"), 1, GL_FALSE, &inverse(camera->viewMatrix())[0][0]);
    // glUniformMatrix4fv(ShaderProgram::getVarLoc("uInvProj"), 1, GL_FALSE, &inverse(uProj)[0][0]);

    // glBindFramebuffer(GL_FRAMEBUFFER, normalFBO);
    // glDisable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glEnable(GL_DEPTH_TEST);
    // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // blurShader.use();

    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, depthColorTex);
    // glUniform1i(ShaderProgram::getVarLoc("depthColorTex"), 0);

    // glActiveTexture(GL_TEXTURE1);
    // glBindTexture(GL_TEXTURE_2D, normalTex);
    // glUniform1i(ShaderProgram::getVarLoc("normalTex"), 1);
    
    // glUniform2f(ShaderProgram::getVarLoc("viewport"), app->width(), app->height());

    // glUniformMatrix4fv(ShaderProgram::getVarLoc("uInvView"), 1, GL_FALSE, &inverse(camera->viewMatrix())[0][0]);
    // glUniformMatrix4fv(ShaderProgram::getVarLoc("uInvProj"), 1, GL_FALSE, &inverse(uProj)[0][0]);

    // glBindFramebuffer(GL_FRAMEBUFFER, blurredFBO);
    // glDisable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glEnable(GL_DEPTH_TEST);
    // glClear(GL_COLOR_BUFFER_BIT);
    // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // waterShader.use();

    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, cumulativeDepthTex);
    // glUniform1i(ShaderProgram::getVarLoc("cumulativeDepthTex"), 0);

    // glActiveTexture(GL_TEXTURE1);
    // glBindTexture(GL_TEXTURE_2D, blurredTex);
    // glUniform1i(ShaderProgram::getVarLoc("normalTex"), 1);

    // glActiveTexture(GL_TEXTURE2);
    // glBindTexture(GL_TEXTURE_2D, depthColorTex);
    // glUniform1i(ShaderProgram::getVarLoc("depthTex"), 2);
    
    // glUniformMatrix4fv(ShaderProgram::getVarLoc("uInvView"), 1, GL_FALSE, &inverse(camera->viewMatrix())[0][0]);
    // glUniformMatrix4fv(ShaderProgram::getVarLoc("uInvProj"), 1, GL_FALSE, &inverse(uProj)[0][0]);
    // glUniform3f(ShaderProgram::getVarLoc("lookDir"), camera->lookDir().x, camera->lookDir().y, camera->lookDir().z);
    // glUniform3f(ShaderProgram::getVarLoc("cameraPos"), camera->position().x, camera->position().y, camera->position().z);

    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // glClear(GL_COLOR_BUFFER_BIT);
    // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    if (m_drawGizmos){
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glEnable(GL_DEPTH_TEST);
        if (auto lockedSolver = m_solverManager.getSolver().lock()){
            lockedSolver->accept(m_solverGizmos);
        }
        m_solverGizmos.render();
    }
}

void ParticleRenderer3D::reload(){
    m_particleShader.reload();
    // cumulativeParticleShader.reload();
    // gridShader.reload();
    // normalShader.reload();
    // blurShader.reload();
    // waterShader.reload();

    initOpenGL();
}