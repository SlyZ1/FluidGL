#include "smokeFlipSolverGPU.hpp"

using namespace std;
using namespace glm;

void SmokeFlipSolverGPU::deleteBuffers(){
    ShaderProgram::deleteBuffer(m_smokeDensityBuffer);
    ShaderProgram::deleteBuffer(m_temperatureBuffer);
    ShaderProgram::deleteBuffer(m_oldDensityBuffer);
    ShaderProgram::deleteBuffer(m_oldTemperatureBuffer);

    ShaderProgram::deleteTexture(m_gridDensityTexture);
    ShaderProgram::deleteBuffer(m_gridDensityWeightsBuffer);
}

void SmokeFlipSolverGPU::createBuffers(){
    m_allocatedSSBOData += ShaderProgram::genSSBO<float>(m_smokeDensityBuffer, m_config.getPartN(), 0.0f);
    m_allocatedSSBOData += ShaderProgram::genSSBO<float>(m_temperatureBuffer, m_config.getPartN(), 0.0f);
    m_allocatedSSBOData += ShaderProgram::genSSBO<float>(m_oldDensityBuffer, m_config.getPartN(), 0.0f);
    m_allocatedSSBOData += ShaderProgram::genSSBO<float>(m_oldTemperatureBuffer, m_config.getPartN(), 0.0f);

    m_allocatedTextureData += ShaderProgram::genTexture(m_gridDensityTexture, m_config.getGridDim());
    m_allocatedSSBOData += ShaderProgram::genSSBO<float>(m_gridDensityWeightsBuffer, m_config.getGridX() * m_config.getGridY() * m_config.getGridZ(), 0.0f);

    cout << m_allocatedTextureData << endl;

    m_stats->setStorage(m_allocatedDataStatIndex, m_allocatedSSBOData);
    m_stats->setStorage(m_allocatedTextureDataStatIndex, m_allocatedTextureData);
}


SmokeFlipSolverGPU::SmokeFlipSolverGPU(SmokeFlipSolverGPUConfig config) 
: FlipSolverGPU(make_shared<SmokeFlipSolverGPUConfig>(move(config))), m_config(static_cast<SmokeFlipSolverGPUConfig&>(*m_baseConfig)) {
    m_isFull = true;
    m_collideWithObstacle = false;

    m_allocatedTextureDataStatIndex = m_stats->registerStorage("Allocated Texture Data");

    ShaderProgram::loadCompute(m_integrateShader, "src/shaders/FLIP/statesmoke/integrateparticles.glsl");
    ShaderProgram::loadCompute(m_sortExtraAttributesShader, "src/shaders/FLIP/pushappart/sortextraattributes.glsl");

    createBuffers();
}

SmokeFlipSolverGPU::~SmokeFlipSolverGPU() {
    deleteBuffers();
}

void SmokeFlipSolverGPU::reload(){
    FlipSolverGPU::reload();

    m_config = static_cast<SmokeFlipSolverGPUConfig&>(*m_baseConfig);
    
    m_integrateShader.reload();
    m_sortExtraAttributesShader.reload();

    createBuffers();
}

void SmokeFlipSolverGPU::integrateParticles() {
    ShaderProgram::SSBOBarrier();

    vec3 minPos = cellToPos(0, m_config.getGridX(), m_config.getGridY(), m_config.getGridZ());
    vec3 maxPos = cellToPos(m_config.getGridX() * m_config.getGridY() * m_config.getGridZ() - 1, m_config.getGridX(), m_config.getGridY(), m_config.getGridZ());

    m_integrateShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, *getPosBuffer());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, *getVelBuffer());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, getVelXBuffer());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, getVelYBuffer());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, getVelZBuffer());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, getIsAirBuffer());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, *getPartNBuffer());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, m_smokeDensityBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, m_temperatureBuffer);
    glUniform1f(ShaderProgram::getVarLoc("dt"), m_config.getDt());

    glUniform3f(ShaderProgram::getVarLoc("minPos"), minPos.x, minPos.y, minPos.z);
    glUniform3f(ShaderProgram::getVarLoc("maxPos"), maxPos.x, maxPos.y, maxPos.z);
    vec3 minSource = m_config.getDensitySource().min(m_config.getPartRadius());
    vec3 maxSource = m_config.getDensitySource().max(m_config.getPartRadius());
    glUniform3f(ShaderProgram::getVarLoc("minDensitySourcePos"), minSource.x, minSource.y, minSource.z);
    glUniform3f(ShaderProgram::getVarLoc("maxDensitySourcePos"), maxSource.x, maxSource.y, maxSource.z);

    glUniform1f(ShaderProgram::getVarLoc("flotAlpha"), m_config.getFlotAlpha());
    glUniform1f(ShaderProgram::getVarLoc("flotBeta"), m_config.getFlotBeta());
    glUniform1f(ShaderProgram::getVarLoc("densityDecreaseRate"), m_config.getDensityDecreaseRate());
    glUniform1f(ShaderProgram::getVarLoc("temperatureDecreaseRate"), m_config.getTemperatureDecreaseRate());

    glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config.getGridX());
    glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config.getGridY());
    glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config.getGridZ());
    glUniform1f(ShaderProgram::getVarLoc("h"), m_config.getH());

    m_integrateShader.dispatch((m_config.getPartN() + 255) / 256);
}

void SmokeFlipSolverGPU::integrateGrid() {}

void SmokeFlipSolverGPU::sortAttribute(GLuint cellParticleIdsBuffer, GLuint attributeBuffer, GLuint sortedAttributeBuffer){
    m_sortExtraAttributesShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, cellParticleIdsBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, attributeBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, sortedAttributeBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, *getPartNBuffer());
    glUniform1i(ShaderProgram::getVarLoc("configPartN"), m_config.getPartN());
    m_sortExtraAttributesShader.dispatch((m_config.getPartN() + 255) / 256);
}

void SmokeFlipSolverGPU::sortExtraAttributes(GLuint cellParticleIdsBuffer){
    ShaderProgram::SSBOBarrier();

    swap(m_smokeDensityBuffer, m_oldDensityBuffer);
    swap(m_temperatureBuffer, m_oldTemperatureBuffer);
    
    sortAttribute(cellParticleIdsBuffer, m_oldDensityBuffer, m_smokeDensityBuffer);
    sortAttribute(cellParticleIdsBuffer, m_oldTemperatureBuffer, m_temperatureBuffer);

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

    glBindBuffer(GL_COPY_READ_BUFFER, m_smokeDensityBuffer);
    glBindBuffer(GL_COPY_WRITE_BUFFER, m_oldDensityBuffer);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, m_config.getPartN() * sizeof(float));
    glBindBuffer(GL_COPY_READ_BUFFER, m_temperatureBuffer);
    glBindBuffer(GL_COPY_WRITE_BUFFER, m_oldTemperatureBuffer);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, m_config.getPartN() * sizeof(float));

    attributeToGrid(m_smokeDensityBuffer, m_gridDensityTexture, m_gridDensityWeightsBuffer);
}