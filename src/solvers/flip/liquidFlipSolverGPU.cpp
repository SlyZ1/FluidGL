#include "liquidFlipSolverGPU.hpp"

using namespace std;
using namespace glm;

void LiquidFlipSolverGPU::deleteBuffers(){}

void LiquidFlipSolverGPU::createBuffers(){}


LiquidFlipSolverGPU::LiquidFlipSolverGPU(LiquidFlipSolverGPUConfig config) 
: FlipSolverGPU(make_shared<LiquidFlipSolverGPUConfig>(move(config))), m_config(static_cast<LiquidFlipSolverGPUConfig&>(*m_baseConfig)) {

    ShaderProgram::loadCompute(m_integrateShader, "src/shaders/FLIP/stateliquid/integrateparticles.glsl");

    createBuffers();
}

LiquidFlipSolverGPU::~LiquidFlipSolverGPU() {
    deleteBuffers();
}

void LiquidFlipSolverGPU::reload(){
    FlipSolverGPU::reload();

    m_config = static_cast<LiquidFlipSolverGPUConfig&>(*m_baseConfig);
    
    m_integrateShader.reload();

    createBuffers();
}

void LiquidFlipSolverGPU::integrateParticles() {
    ShaderProgram::SSBOBarrier();

    vec3 minPos = cellToPos(0, m_config.getGridX(), m_config.getGridY(), m_config.getGridZ());
    vec3 maxPos = cellToPos(m_config.getGridX() * m_config.getGridY() * m_config.getGridZ() - 1, m_config.getGridX(), m_config.getGridY(), m_config.getGridZ());

    m_integrateShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, *getPosBuffer());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, *getVelBuffer());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, *getPartNBuffer());
    glUniform1f(ShaderProgram::getVarLoc("dt"), m_config.getDt());
    glUniform3f(ShaderProgram::getVarLoc("minPos"), minPos.x, minPos.y, minPos.z);
    glUniform3f(ShaderProgram::getVarLoc("maxPos"), maxPos.x, maxPos.y, maxPos.z);
    glUniform1f(ShaderProgram::getVarLoc("gravity"), m_config.getGravity());
    m_integrateShader.dispatch((m_config.getPartN() + 255) / 256);
}

void LiquidFlipSolverGPU::integrateGrid() {}