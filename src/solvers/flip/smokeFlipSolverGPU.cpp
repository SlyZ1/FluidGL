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

    ShaderProgram::deleteBuffer(m_vorticityBuffer);
    ShaderProgram::deleteBuffer(m_vorticityForceBuffer);
}

void SmokeFlipSolverGPU::createBuffers(){
    m_allocatedSSBOData += ShaderProgram::genSSBO<float>(m_smokeDensityBuffer, m_config.getPartN(), 0.0f);
    m_allocatedSSBOData += ShaderProgram::genSSBO<float>(m_temperatureBuffer, m_config.getPartN(), 0.0f);
    m_allocatedSSBOData += ShaderProgram::genSSBO<float>(m_oldDensityBuffer, m_config.getPartN(), 0.0f);
    m_allocatedSSBOData += ShaderProgram::genSSBO<float>(m_oldTemperatureBuffer, m_config.getPartN(), 0.0f);

    m_allocatedTextureData += ShaderProgram::genTexture(m_gridDensityTexture, m_config.getGridDim());
    m_allocatedSSBOData += ShaderProgram::genSSBO<float>(m_gridDensityWeightsBuffer, m_config.getGridX() * m_config.getGridY() * m_config.getGridZ(), 0.0f);

    m_allocatedSSBOData += ShaderProgram::genSSBO<vec4>(m_vorticityBuffer, m_config.getGridX()*m_config.getGridY()*m_config.getGridZ(), vec4(0.0f));
    m_allocatedSSBOData += ShaderProgram::genSSBO<vec4>(m_vorticityForceBuffer, m_config.getGridX()*m_config.getGridY()*m_config.getGridZ(), vec4(0.0f));

    m_stats->setStorage(m_allocatedDataStatIndex, m_allocatedSSBOData);
    m_stats->setStorage(m_allocatedTextureDataStatIndex, m_allocatedTextureData);
}


SmokeFlipSolverGPU::SmokeFlipSolverGPU(SmokeFlipSolverGPUConfig config) 
: FlipSolverGPU(make_shared<SmokeFlipSolverGPUConfig>(move(config))), m_config(static_cast<SmokeFlipSolverGPUConfig&>(*m_baseConfig)) {
    m_isFull = !(any(m_config.getOpenBoundsX()) || any(m_config.getOpenBoundsY()) || any(m_config.getOpenBoundsZ()));
    m_collideWithObstacle = false;

    m_allocatedTextureDataStatIndex = m_stats->registerStorage("Allocated Texture Data");

    ShaderProgram::loadCompute(m_integrateShader, "src/shaders/FLIP/statesmoke/integrateparticles.glsl");
    ShaderProgram::loadCompute(m_integrateGridShader, "src/shaders/FLIP/statesmoke/integrategrid.glsl");
    ShaderProgram::loadCompute(m_sortExtraAttributesShader, "src/shaders/FLIP/pushappart/sortextraattributes.glsl");

    ShaderProgram::loadCompute(m_compensateNumPartShader, "src/shaders/FLIP/statesmoke/compensatenumpart.glsl");
    ShaderProgram::loadCompute(m_diffuseFieldsShader, "src/shaders/FLIP/statesmoke/diffusefields.glsl");

    ShaderProgram::loadCompute(m_computeVorticityShader, "src/shaders/FLIP/statesmoke/computevorticity.glsl");
    ShaderProgram::loadCompute(m_computeVorticityForceShader, "src/shaders/FLIP/statesmoke/computevorticityforce.glsl");

    createBuffers();
}

SmokeFlipSolverGPU::~SmokeFlipSolverGPU() {
    deleteBuffers();
}

void SmokeFlipSolverGPU::reload(){
    FlipSolverGPU::reload();

    m_integrationFrame = 0;

    m_config = static_cast<SmokeFlipSolverGPUConfig&>(*m_baseConfig);

    m_isFull = !(any(m_config.getOpenBoundsX()) || any(m_config.getOpenBoundsY()) || any(m_config.getOpenBoundsZ()));
    
    m_integrateShader.reload();
    m_integrateGridShader.reload();
    m_sortExtraAttributesShader.reload();
    m_compensateNumPartShader.reload();
    m_diffuseFieldsShader.reload();
    m_computeVorticityShader.reload();
    m_computeVorticityForceShader.reload();

    createBuffers();
}

void SmokeFlipSolverGPU::integrateParticles() {
    m_integrationFrame++;

    ShaderProgram::SSBOBarrier();

    if (m_config.getDiffusionFactor() > 0){
        m_diffuseFieldsShader.use();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, *getPosBuffer());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, getFirstCellParticleBuffer());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_smokeDensityBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_temperatureBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_oldDensityBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_oldTemperatureBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, *getPartNBuffer());
    
        glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config.getGridX());
        glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config.getGridY());
        glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config.getGridZ());
        glUniform1f(ShaderProgram::getVarLoc("h"), m_config.getH());
        glUniform1f(ShaderProgram::getVarLoc("pph"), m_config.getPartPerH());
        glUniform1f(ShaderProgram::getVarLoc("dt"), m_config.getDt());
        glUniform1f(ShaderProgram::getVarLoc("kappa"), m_config.getDiffusionFactor() * 0.001f);
        m_diffuseFieldsShader.dispatch((m_config.getPartN() + 255) / 256);
    
        ShaderProgram::SSBOBarrier();
    }

    if (any(m_config.getOpenBoundsX()) || any(m_config.getOpenBoundsY()) || any(m_config.getOpenBoundsZ())) {
        m_compensateNumPartShader.use();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, *getPosBuffer());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, *getVelBuffer());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_smokeDensityBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_temperatureBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, getFirstCellParticleBuffer());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, *getPartNBuffer());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, getIsDeadBuffer());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, getVelXBuffer());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, getVelYBuffer());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, getVelZBuffer());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, getIsAirBuffer());
    
        glUniform1i(ShaderProgram::getVarLoc("configPartN"), m_config.getPartN());
        glUniform1ui(ShaderProgram::getVarLoc("frame"), m_integrationFrame);
        glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config.getGridX());
        glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config.getGridY());
        glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config.getGridZ());
        glUniform1f(ShaderProgram::getVarLoc("h"), m_config.getH());
        glUniform1f(ShaderProgram::getVarLoc("pph"), m_config.getPartPerH());
    
        glUniform2i(ShaderProgram::getVarLoc("openBoundsX"), m_config.getOpenBoundsX().x ? 1 : 0, m_config.getOpenBoundsX().y ? 1 : 0);
        glUniform2i(ShaderProgram::getVarLoc("openBoundsY"), m_config.getOpenBoundsY().x ? 1 : 0, m_config.getOpenBoundsY().y ? 1 : 0);
        glUniform2i(ShaderProgram::getVarLoc("openBoundsZ"), m_config.getOpenBoundsZ().x ? 1 : 0, m_config.getOpenBoundsZ().y ? 1 : 0);
    
        vec3 minPos = cellToPos(0, m_config.getGridX(), m_config.getGridY(), m_config.getGridZ());
        glUniform3f(ShaderProgram::getVarLoc("minPos"), minPos.x, minPos.y, minPos.z);
        m_compensateNumPartShader.dispatch((m_config.getGridDim() + 7) / 8);
    
        ShaderProgram::SSBOBarrier();
    }

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

    vec3 minPos = cellToPos(0, m_config.getGridX(), m_config.getGridY(), m_config.getGridZ());
    vec3 maxPos = cellToPos(m_config.getGridX() * m_config.getGridY() * m_config.getGridZ() - 1, m_config.getGridX(), m_config.getGridY(), m_config.getGridZ());

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

void SmokeFlipSolverGPU::integrateGrid() {
    if (m_config.getVorticityFactor() > 0){
        ShaderProgram::SSBOBarrier();
    
        m_computeVorticityShader.use();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, getVelXBuffer());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, getVelYBuffer());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, getVelZBuffer());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_vorticityBuffer);
        glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config.getGridX());
        glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config.getGridY());
        glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config.getGridZ());
        glUniform1f(ShaderProgram::getVarLoc("h"), m_config.getH());
        m_computeVorticityShader.dispatch((m_config.getGridDim() + 7) / 8);
    
        ShaderProgram::SSBOBarrier();
    
        m_computeVorticityForceShader.use();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_vorticityBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_vorticityForceBuffer);
        glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config.getGridX());
        glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config.getGridY());
        glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config.getGridZ());
        glUniform1f(ShaderProgram::getVarLoc("h"), m_config.getH());
        glUniform1f(ShaderProgram::getVarLoc("eps"), m_config.getVorticityFactor());
        m_computeVorticityForceShader.dispatch((m_config.getGridDim() + 7) / 8);
    }

    ShaderProgram::SSBOBarrier();

    m_integrateGridShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, getVelXBuffer());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, getVelYBuffer());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, getVelZBuffer());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_vorticityForceBuffer);
    glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config.getGridX());
    glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config.getGridY());
    glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config.getGridZ());
    glUniform1f(ShaderProgram::getVarLoc("h"), m_config.getH());
    glUniform1f(ShaderProgram::getVarLoc("dt"), m_config.getDt());

    m_integrateGridShader.dispatch((m_config.getGridDim() + 7) / 8);
}

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