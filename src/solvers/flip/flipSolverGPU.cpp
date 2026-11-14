#include "flipSolverGPU.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include "helpers/utils.hpp"

using namespace glm; 
using namespace std;

void FlipSolverGPU::deleteBuffers(){
    ShaderProgram::deleteBuffer(m_rXBuffer); ShaderProgram::deleteBuffer(m_rYBuffer); ShaderProgram::deleteBuffer(m_rZBuffer);
    ShaderProgram::deleteBuffer(m_velXBuffer); ShaderProgram::deleteBuffer(m_velYBuffer); ShaderProgram::deleteBuffer(m_velZBuffer);
    ShaderProgram::deleteBuffer(m_oldVelXBuffer); ShaderProgram::deleteBuffer(m_oldVelYBuffer); ShaderProgram::deleteBuffer(m_oldVelZBuffer);
    ShaderProgram::deleteBuffer(m_partPosBuffer); ShaderProgram::deleteBuffer(m_partVelBuffer); 
    ShaderProgram::deleteBuffer(m_oldPartPosBuffer); ShaderProgram::deleteBuffer(m_oldPartVelBuffer);
    ShaderProgram::deleteBuffer(m_isAirBuffer); 

    ShaderProgram::deleteBuffer(m_isDeadBuffer); ShaderProgram::deleteBuffer(m_aliveNBuffer);

    for (GLuint buffer : m_blockSumBuffers) ShaderProgram::deleteBuffer(buffer);
    for (GLuint buffer : m_zBuffers) ShaderProgram::deleteBuffer(buffer);
    for (GLuint buffer : m_rBuffers) ShaderProgram::deleteBuffer(buffer);
    for (GLuint buffer : m_coarseIsAirBuffers) ShaderProgram::deleteBuffer(buffer);
    m_blockSumBuffers.clear();
    m_zBuffers.clear();
    m_rBuffers.clear();
    m_coarseIsAirBuffers.clear();

    ShaderProgram::deleteBuffer(m_cellOfBuffer);
    ShaderProgram::deleteBuffer(m_firstCellParticleBuffer); ShaderProgram::deleteBuffer(m_cellParticleIdsBuffer);
    ShaderProgram::deleteBuffer(m_firstCellParticleBuffer2);

    ShaderProgram::deleteBuffer(m_minusDivBuffer); ShaderProgram::deleteBuffer(m_pressureBuffer);

    ShaderProgram::deleteBuffer(m_rhoBuffer); ShaderProgram::deleteBuffer(m_smoothRhoBuffer); ShaderProgram::deleteBuffer(m_gradRhoBuffer);
    ShaderProgram::deleteBuffer(m_curvatureBuffer);
}

void FlipSolverGPU::createBuffers(){
    deleteBuffers();

    m_allocatedSSBOData = 0;

    FluidInitializer initializer = m_config->getFluidInitializer();

    vec3 boundsDim = initializer.bounds(m_config->getPartRadius()) / m_config->getH();
    
    int nx = (int)std::floor(boundsDim.x * m_config->getPartPerH() * m_config->getDensityMultiplier());
    int ny = (int)std::floor(boundsDim.y * m_config->getPartPerH() * m_config->getDensityMultiplier());
    int nz = (int)std::floor(boundsDim.z * m_config->getPartPerH() * m_config->getDensityMultiplier());
    int partN = nx * ny * nz;

    m_config->setPartN(partN);

    m_stats->setCounter(m_numPartStatIndex, partN);
    m_stats->setCounter(m_numCellsStatIndex, m_config->getGridX() * m_config->getGridY() * m_config->getGridZ());

    vector<vec4> partVel = vector<vec4>(partN, vec4(0.f));
    vector<vec4> partPos = vector<vec4>(partN, vec4(0.f));

    int i = 0;
    for (int x = 0; x < nx; x++)
        for (int y = 0; y < ny; y++)
            for (int z = 0; z < nz; z++) {
                if (i >= partN) continue;
                partPos[i] = (vec4(x,y,z,1) + vec4(1, 0, 1, 0) * 0.5f * (float)((int)y % 2) - vec4(nx,ny,nz,1) * 0.5f) 
                                * m_config->getH() / m_config->getPartPerH() / m_config->getDensityMultiplier();

                partPos[i] += vec4(initializer.pos(m_config->getPartRadius()), 1);
                i++;
            }

    m_allocatedSSBOData += ShaderProgram::genSSBO<float>(m_rXBuffer, (m_config->getGridX() + 1) * m_config->getGridY() * m_config->getGridZ(), 0.0f);
    m_allocatedSSBOData += ShaderProgram::genSSBO<float>(m_velXBuffer, (m_config->getGridX() + 1) * m_config->getGridY() * m_config->getGridZ(), 0.0f);
    m_allocatedSSBOData += ShaderProgram::genSSBO<float>(m_oldVelXBuffer, (m_config->getGridX() + 1) * m_config->getGridY() * m_config->getGridZ(), 0.0f);

    m_allocatedSSBOData += ShaderProgram::genSSBO<float>(m_rYBuffer, m_config->getGridX() * (m_config->getGridY() + 1) * m_config->getGridZ(), 0.0f);
    m_allocatedSSBOData += ShaderProgram::genSSBO<float>(m_velYBuffer, m_config->getGridX() * (m_config->getGridY() + 1) * m_config->getGridZ(), 0.0f);
    m_allocatedSSBOData += ShaderProgram::genSSBO<float>(m_oldVelYBuffer, m_config->getGridX() * (m_config->getGridY() + 1) * m_config->getGridZ(), 0.0f);

    m_allocatedSSBOData += ShaderProgram::genSSBO<float>(m_rZBuffer, m_config->getGridX() * m_config->getGridY() * (m_config->getGridZ() + 1), 0.0f);
    m_allocatedSSBOData += ShaderProgram::genSSBO<float>(m_velZBuffer, m_config->getGridX() * m_config->getGridY() * (m_config->getGridZ() + 1), 0.0f);
    m_allocatedSSBOData += ShaderProgram::genSSBO<float>(m_oldVelZBuffer, m_config->getGridX() * m_config->getGridY() * (m_config->getGridZ() + 1), 0.0f);

    m_allocatedSSBOData += ShaderProgram::genDataSSBO<vec4>(m_partPosBuffer, m_config->getPartN(), partPos.data());
    m_allocatedSSBOData += ShaderProgram::genDataSSBO<vec4>(m_oldPartPosBuffer, m_config->getPartN(), partPos.data());

    m_allocatedSSBOData += ShaderProgram::genSSBO<vec4>(m_partVelBuffer, m_config->getPartN(), vec4(0.0f));
    m_allocatedSSBOData += ShaderProgram::genSSBO<vec4>(m_oldPartVelBuffer, m_config->getPartN(), vec4(0.0f));

    m_allocatedSSBOData += ShaderProgram::genSSBO<uint>(m_isAirBuffer, m_config->getGridX() * m_config->getGridY() * m_config->getGridZ(), 0);

    uint uintPartN = (uint)partN;
    m_allocatedSSBOData += ShaderProgram::genSSBO<uint>(m_isDeadBuffer, partN, 0);
    m_allocatedSSBOData += ShaderProgram::genDataSSBO<uint>(m_aliveNBuffer, 1, &uintPartN);

    int ceiledN = (int)glm::ceil((float)(m_config->getGridX() * m_config->getGridY() * m_config->getGridZ() + 1) / 512.f) * 512;
    int n = ceiledN;
    do {
        n = (n + 511) / 512;
        GLuint blockSumBuffer = 0;
        m_allocatedSSBOData += ShaderProgram::genSSBO<uint>(blockSumBuffer, n, 0);
        m_blockSumBuffers.push_back(blockSumBuffer);
    } while (n >= SHARED_SIZE);

    m_allocatedSSBOData += ShaderProgram::genSSBO<uint>(m_cellOfBuffer, partN, 0);
    m_allocatedSSBOData += ShaderProgram::genSSBO<uint>(m_firstCellParticleBuffer, ceiledN, 0);
    m_allocatedSSBOData += ShaderProgram::genSSBO<uint>(m_firstCellParticleBuffer2, ceiledN, 0);
    m_allocatedSSBOData += ShaderProgram::genSSBO<uint>(m_cellParticleIdsBuffer, partN, 0);

    ivec3 dim = m_config->getGridDim();
    bool genBuffers = true;
    while (genBuffers) {
        genBuffers = min({dim.x, dim.y, dim.z}) > 4;
        GLuint zBuffer = 0;
        GLuint rBuffer = 0;
        GLuint isAirBuffer = 0;
        int n = dim.x * dim.y * dim.z;
        m_allocatedSSBOData += ShaderProgram::genSSBO<float>(zBuffer, n, 0.0f);
        m_allocatedSSBOData += ShaderProgram::genSSBO<float>(rBuffer, n, 0.0f);
        m_allocatedSSBOData += ShaderProgram::genSSBO<uint>(isAirBuffer, n, 0);
        m_zBuffers.push_back(zBuffer);
        m_rBuffers.push_back(rBuffer);
        m_coarseIsAirBuffers.push_back(isAirBuffer);

        dim = (dim + 1) / 2;
    }
    
    m_allocatedSSBOData += ShaderProgram::genSSBO<float>(m_minusDivBuffer, m_config->getGridX() * m_config->getGridY() * m_config->getGridZ(), 0.0f);
    m_allocatedSSBOData += ShaderProgram::genSSBO<float>(m_pressureBuffer, m_config->getGridX() * m_config->getGridY() * m_config->getGridZ(), 0.0f);

    m_allocatedSSBOData += ShaderProgram::genSSBO<float>(m_rhoBuffer, m_config->getGridX() * m_config->getGridY() * m_config->getGridZ(), 0.0f);
    m_allocatedSSBOData += ShaderProgram::genSSBO<float>(m_smoothRhoBuffer, m_config->getGridX() * m_config->getGridY() * m_config->getGridZ(), 0.0f);
    m_allocatedSSBOData += ShaderProgram::genSSBO<float>(m_curvatureBuffer, m_config->getGridX() * m_config->getGridY() * m_config->getGridZ(), 0.0f);
    m_allocatedSSBOData += ShaderProgram::genSSBO<vec4>(m_gradRhoBuffer, m_config->getGridX() * m_config->getGridY() * m_config->getGridZ(), vec4(0.0f));

    m_stats->setStorage(m_allocatedDataStatIndex, m_allocatedSSBOData);
}

void FlipSolverGPU::initCGS(){
    m_cgs.init(m_config->getGridX() * m_config->getGridY() * m_config->getGridZ(), 0, m_minusDivBuffer, m_pressureBuffer);

    std::function<void(GLuint, GLuint, GLuint, int, bool)> sparseMatVec =
    [this](GLuint, GLuint dBuf, GLuint AdBuf, int, bool dispatch) {
        m_sparseMatVecShader.use();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, dBuf);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_isAirBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, AdBuf);
        glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config->getGridX());
        glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config->getGridY());
        glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config->getGridZ());
        glUniform1f(ShaderProgram::getVarLoc("h"), m_config->getH());
        if (dispatch){
            m_sparseMatVecShader.dispatch((m_config->getGridDim() + 7) / 8);
        }
    };
    m_cgs.setCustomMatVec(sparseMatVec);
    m_cgs.setAccuracyFirst(m_config->getAccuracyFirst());
    m_cgs.setPrecond([this](GLuint zBuffer, GLuint rBuffer, bool dispatch) {
        MGPrecond(zBuffer, rBuffer, dispatch);
    });

    uvec3 dim = m_config->getGridDim();
    vector<DispatchParams> params = {};
    vector<DispatchParams> linearParams = {};
    for (int i = 0; i < (int)m_zBuffers.size(); i++)
    {
        linearParams.push_back((DispatchParams){(dim.x * dim.y * dim.z + 255) / 256, 1, 1});
        params.push_back((DispatchParams){(dim.x + 7u) / 8u, (dim.y + 7u) / 8u, (dim.z + 7u) / 8u});
        dim = (dim + 1u) / 2u;
    }
    m_indirectParamsIndex = m_cgs.addDispatchParams(params);
    m_cgs.addDispatchParams(linearParams);
    m_indirectBuffer = m_cgs.getIndirectBuffer();
}

FlipSolverGPU::FlipSolverGPU(shared_ptr<FlipSolverGPUConfig> config) 
: IParticleSolver(move(config), "FLIP GPU Solver"), m_config(static_pointer_cast<FlipSolverGPUConfig>(m_baseConfig)) {
    m_integrationStatIndex        = m_stats->registerTimer("Integration");
    m_pushAppartStatIndex         = m_stats->registerTimer("Push Appart");
    m_collisionStatIndex          = m_stats->registerTimer("Collision");
    m_p2gStatIndex                = m_stats->registerTimer("P2G");
    m_surfaceTensionStatIndex     = m_stats->registerTimer("Surface Tension");
    m_incompressibilityStatIndex  = m_stats->registerTimer("Incompressibility");
    m_g2pStatIndex                = m_stats->registerTimer("G2P");
    m_scanStatIndex               = m_stats->registerTimer("Counting Sort");

    m_numPartStatIndex = m_stats->registerCounter("Num Particles");
    m_numCellsStatIndex = m_stats->registerCounter("Num Cells");

    m_integrateTimer.init();
    m_collisionTimer.init();
    m_pushAppartTimer.init();
    m_p2gTimer.init();
    m_surfaceTensionTimer.init();
    m_incompressibilityTimer.init();
    m_g2pTimer.init();
    m_scanTimer.init();

    ShaderProgram::loadCompute(m_collisionShader, "src/shaders/FLIP/collisions.glsl");
    ShaderProgram::loadCompute(m_resetUintBuffersShader, "src/shaders/FLIP/prefixsum/resetuintbuffer.glsl");
    ShaderProgram::loadCompute(m_partCountShader, "src/shaders/FLIP/pushappart/partcount.glsl");
    ShaderProgram::loadCompute(m_localSumShader, "src/shaders/FLIP/prefixsum/localsum.glsl");
    ShaderProgram::loadCompute(m_smallSumShader, "src/shaders/FLIP/prefixsum/smallsum.glsl");
    ShaderProgram::loadCompute(m_globalSumShader, "src/shaders/FLIP/prefixsum/globalsum.glsl");
    ShaderProgram::loadCompute(m_cellParticleIdShader, "src/shaders/FLIP/pushappart/cellparticleid.glsl");
    ShaderProgram::loadCompute(m_resetBuffersShader, "src/shaders/FLIP/pushappart/resetbuffers.glsl");
    ShaderProgram::loadCompute(m_getCorrectionsShader, "src/shaders/FLIP/pushappart/getcorrections.glsl");
    ShaderProgram::loadCompute(m_applyCorrectionsShader, "src/shaders/FLIP/pushappart/applycorrections.glsl");
    ShaderProgram::loadCompute(m_sortBuffers, "src/shaders/FLIP/pushappart/sortbuffers.glsl");
    ShaderProgram::loadCompute(m_a2gShader, "src/shaders/FLIP/p2g/a2g.glsl");
    ShaderProgram::loadCompute(m_applyWieghtsAttributesShader, "src/shaders/FLIP/p2g/applyweightsattributes.glsl");
    ShaderProgram::loadCompute(m_p2gShader, "src/shaders/FLIP/p2g/p2g.glsl");
    ShaderProgram::loadCompute(m_resetFloatBufferShader, "src/shaders/FLIP/p2g/resetfloatbuffer.glsl");
    ShaderProgram::loadCompute(m_applyWeightsShader, "src/shaders/FLIP/p2g/applyweights.glsl");
    ShaderProgram::loadCompute(m_solveIncompressibilityShader, "src/shaders/FLIP/solveincompressibility.glsl");
    ShaderProgram::loadCompute(m_g2pShader, "src/shaders/FLIP/g2p.glsl");
    ShaderProgram::loadCompute(m_mgCoarseIsAir, "src/shaders/FLIP/incompressibility/multigrid/coarseisair.glsl");
    ShaderProgram::loadCompute(m_mgSmooth, "src/shaders/FLIP/incompressibility/multigrid/smooth.glsl");
    ShaderProgram::loadCompute(m_mgRestrict, "src/shaders/FLIP/incompressibility/multigrid/restrict.glsl");
    ShaderProgram::loadCompute(m_mgProlongate, "src/shaders/FLIP/incompressibility/multigrid/prolongate.glsl");
    ShaderProgram::loadCompute(m_sparseMatVecShader, "src/shaders/FLIP/incompressibility/matvec.glsl");
    ShaderProgram::loadCompute(m_computeMinusDivShader, "src/shaders/FLIP/incompressibility/computeminusdiv.glsl");
    ShaderProgram::loadCompute(m_pressureToVelShader, "src/shaders/FLIP/incompressibility/pressuretovel.glsl");
    ShaderProgram::loadCompute(m_setAirCellsToZeroShader, "src/shaders/FLIP/incompressibility/setaircellstozero.glsl");
    ShaderProgram::loadCompute(m_computeRhoShader, "src/shaders/FLIP/surfacetension/computerho.glsl");
    ShaderProgram::loadCompute(m_smoothDataShader, "src/shaders/FLIP/surfacetension/smoothdata.glsl");
    ShaderProgram::loadCompute(m_computeGradShader, "src/shaders/FLIP/surfacetension/computegrad.glsl");
    ShaderProgram::loadCompute(m_computeCurvatureShader, "src/shaders/FLIP/surfacetension/computecurvature.glsl");
    ShaderProgram::loadCompute(m_integrateGridShader, "src/shaders/FLIP/integrategrid.glsl");
    createBuffers();

    initCGS();
}

FlipSolverGPU::~FlipSolverGPU() {
    deleteBuffers();
}

void FlipSolverGPU::reload() {
    m_config = static_pointer_cast<FlipSolverGPUConfig>(m_baseConfig);
    m_collisionShader.reload();
    m_resetUintBuffersShader.reload();
    m_partCountShader.reload();
    m_localSumShader.reload();
    m_smallSumShader.reload();
    m_globalSumShader.reload();
    m_cellParticleIdShader.reload();
    m_resetBuffersShader.reload();
    m_getCorrectionsShader.reload();
    m_applyCorrectionsShader.reload();
    m_sortBuffers.reload();
    m_a2gShader.reload();
    m_applyWieghtsAttributesShader.reload();
    m_p2gShader.reload();
    m_resetFloatBufferShader.reload();
    m_applyWeightsShader.reload();
    m_solveIncompressibilityShader.reload();
    m_g2pShader.reload();
    m_mgCoarseIsAir.reload();
    m_mgSmooth.reload();
    m_mgRestrict.reload();
    m_mgProlongate.reload();
    m_sparseMatVecShader.reload();
    m_computeMinusDivShader.reload();
    m_pressureToVelShader.reload();
    m_setAirCellsToZeroShader.reload();
    m_computeRhoShader.reload();
    m_smoothDataShader.reload();
    m_computeGradShader.reload();
    m_computeCurvatureShader.reload();
    m_integrateGridShader.reload();
    createBuffers();
    
    initCGS();

    ISolver::reload();
}

ivec3 FlipSolverGPU::cellToCoord(int cell, int nx, int ny){
    int column = cell % nx;
    int rest = (cell - column) / nx;
    int row = rest % ny;
    int layer = (rest - row) / ny;
    return ivec3(column, row, layer);
}

vec3 FlipSolverGPU::cellToPos(int cell, int nx, int ny, int nz){
    ivec3 coord = cellToCoord(cell, nx, ny);
    vec3 pos = ((vec3)coord - vec3(nx - 1, ny - 1, nz - 1) * 0.5f) * m_config->getH();
    return pos;
}

void FlipSolverGPU::resetUintBuffer(GLuint buffer, int n){
    m_resetUintBuffersShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer);
    glUniform1i(ShaderProgram::getVarLoc("n"), n);
    m_resetUintBuffersShader.dispatch((n + 255) / 256);
}

void FlipSolverGPU::prefixSum(GLuint data, int n, int recursionIndex){
    int newN = (n + 511) / 512;

    // Scan on the local work groups, store the total sum in blockSum
    m_localSumShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_blockSumBuffers[recursionIndex]);
    m_localSumShader.dispatch(newN);

    ShaderProgram::SSBOBarrier();

    // Scan blockSum
    if (newN < SHARED_SIZE){
        m_smallSumShader.use();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_blockSumBuffers[recursionIndex]);
        glUniform1i(ShaderProgram::getVarLoc("length"), newN);
        m_smallSumShader.dispatch(1); 
    }
    else{
        prefixSum(m_blockSumBuffers[recursionIndex], newN, recursionIndex+1);
    }

    ShaderProgram::SSBOBarrier();

    // Sum the scanned blockSum in all local work groups to get the scanned result 
    m_globalSumShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_blockSumBuffers[recursionIndex]);
    glUniform1i(ShaderProgram::getVarLoc("length"), n);
    m_globalSumShader.dispatch(newN);
}

void FlipSolverGPU::countingSort(){
    ShaderProgram::SSBOBarrier();

    // Reset the buffers
    int ceiledN = (int)glm::ceil((float)(m_config->getGridX() * m_config->getGridY() * m_config->getGridZ() + 1) / 512.f) * 512;
    resetUintBuffer(m_firstCellParticleBuffer, ceiledN);
    resetUintBuffer(m_firstCellParticleBuffer2, ceiledN);
    resetUintBuffer(m_cellParticleIdsBuffer, m_config->getPartN());
    resetUintBuffer(m_cellOfBuffer, m_config->getPartN());

    ShaderProgram::SSBOBarrier();

    // Count number of particles in each cell
    m_partCountShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_firstCellParticleBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_cellOfBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_partPosBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_isDeadBuffer);

    glUniform1i(ShaderProgram::getVarLoc("configPartN"), m_config->getPartN());
    glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config->getGridX());
    glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config->getGridY());
    glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config->getGridZ());
    glUniform1f(ShaderProgram::getVarLoc("h"), m_config->getH());
    m_partCountShader.dispatch((m_config->getPartN() + 63) / 64);
    
    ShaderProgram::SSBOBarrier();

    // Exclusive scan of the number of particles
    prefixSum(m_firstCellParticleBuffer, ceiledN);

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    // Copy in another buffer for the next step
    glBindBuffer(GL_COPY_READ_BUFFER, m_firstCellParticleBuffer);
    glBindBuffer(GL_COPY_WRITE_BUFFER, m_firstCellParticleBuffer2);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, ceiledN * sizeof(uint));
    
    ShaderProgram::SSBOBarrier();

    // Get the ordered particle ids grouped by cell 
    m_cellParticleIdShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_firstCellParticleBuffer2);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_cellOfBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_cellParticleIdsBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_isDeadBuffer);

    glUniform1i(ShaderProgram::getVarLoc("configPartN"), m_config->getPartN());
    m_cellParticleIdShader.dispatch((m_config->getPartN() + 63) / 64);

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    swap(m_partPosBuffer, m_oldPartPosBuffer);
    swap(m_partVelBuffer, m_oldPartVelBuffer);
    
    ShaderProgram::SSBOBarrier();

    m_sortBuffers.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_cellParticleIdsBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_oldPartPosBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_oldPartVelBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_partPosBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_partVelBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_isDeadBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, m_aliveNBuffer);

    glUniform1i(ShaderProgram::getVarLoc("configPartN"), m_config->getPartN());

    m_sortBuffers.dispatch((m_config->getPartN() + 255) / 256);

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    glBindBuffer(GL_COPY_READ_BUFFER, m_partPosBuffer);
    glBindBuffer(GL_COPY_WRITE_BUFFER, m_oldPartPosBuffer);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, m_config->getPartN() * sizeof(vec4));
    glBindBuffer(GL_COPY_READ_BUFFER, m_partVelBuffer);
    glBindBuffer(GL_COPY_WRITE_BUFFER, m_oldPartVelBuffer);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, m_config->getPartN() * sizeof(vec4));

    sortExtraAttributes(m_cellParticleIdsBuffer);
}

void FlipSolverGPU::pushAppartParticles(int iterations){
    
    const float minDist = 2.0f * m_config->getPartRadius();
    for (int i = 0; i < iterations; i++)
    {
        countingSort();
        
        ShaderProgram::SSBOBarrier();

        m_getCorrectionsShader.use();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_firstCellParticleBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_cellParticleIdsBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_oldPartPosBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_partPosBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_aliveNBuffer);
        glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config->getGridX());
        glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config->getGridY());
        glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config->getGridZ());
        glUniform1f(ShaderProgram::getVarLoc("h"), m_config->getH());
        glUniform1f(ShaderProgram::getVarLoc("minDist"), minDist);
        m_getCorrectionsShader.dispatch((m_config->getPartN() + 255) / 256);
    }
}

void FlipSolverGPU::particleCollisions(shared_ptr<Mesh> obstacle){
    ShaderProgram::SSBOBarrier();

    m_collisionShader.use();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_partPosBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_partVelBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_isDeadBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_aliveNBuffer);

    vec3 minPos = cellToPos(0, m_config->getGridX(), m_config->getGridY(), m_config->getGridZ());
    vec3 maxPos = cellToPos(m_config->getGridX() * m_config->getGridY() * m_config->getGridZ() - 1, m_config->getGridX(), m_config->getGridY(), m_config->getGridZ());

    glUniform1i(ShaderProgram::getVarLoc("configPartN"), m_config->getPartN());
    glUniform3f(ShaderProgram::getVarLoc("minPos"), minPos.x, minPos.y, minPos.z);
    glUniform3f(ShaderProgram::getVarLoc("maxPos"), maxPos.x, maxPos.y, maxPos.z);
    glUniform1f(ShaderProgram::getVarLoc("radius"), m_config->getPartRadius());
    glUniform1ui(ShaderProgram::getVarLoc("collideWithObstacle"), m_collideWithObstacle);

    glUniform2i(ShaderProgram::getVarLoc("openBoundsX"), m_config->getOpenBoundsX().x ? 1 : 0, m_config->getOpenBoundsX().y ? 1 : 0);
    glUniform2i(ShaderProgram::getVarLoc("openBoundsY"), m_config->getOpenBoundsY().x ? 1 : 0, m_config->getOpenBoundsY().y ? 1 : 0);
    glUniform2i(ShaderProgram::getVarLoc("openBoundsZ"), m_config->getOpenBoundsZ().x ? 1 : 0, m_config->getOpenBoundsZ().y ? 1 : 0);

    if (obstacle){
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, obstacle->getSDF());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, obstacle->getSDFGrad());
    
        glUniform3i(ShaderProgram::getVarLoc("N"), obstacle->getN().x, obstacle->getN().y, obstacle->getN().z);
        vec3 obstacleScale = obstacle->getScale();
        float s = glm::max(obstacleScale.x, glm::max(obstacleScale.y, obstacleScale.z));
        glUniform1f(ShaderProgram::getVarLoc("h"), m_config->getPartRadius() / s);
        Bounds obstacleBounds = obstacle->getSDFBounds();
        glUniform3f(ShaderProgram::getVarLoc("obstacleMinBound"), obstacleBounds.min.x, obstacleBounds.min.y, obstacleBounds.min.z);
        glUniform3f(ShaderProgram::getVarLoc("obstacleMaxBound"), obstacleBounds.max.x, obstacleBounds.max.y, obstacleBounds.max.z);
        glUniform3f(ShaderProgram::getVarLoc("obstacleVelocity"), obstacle->getVel().x, obstacle->getVel().y, obstacle->getVel().z);
        glUniformMatrix4fv(ShaderProgram::getVarLoc("obstacleInverseMatrix"), 1, GL_FALSE, &inverse(obstacle->getModelMatrix())[0][0]);
    }
    else {
        glUniform3f(ShaderProgram::getVarLoc("obstacleMinBound"), 0.0f, 0.0f, 0.0f);
        glUniform3f(ShaderProgram::getVarLoc("obstacleMaxBound"), 0.0f, 0.0f, 0.0f);
    }

    m_collisionShader.dispatch((m_config->getPartN() + 255) / 256);
}

void FlipSolverGPU::resetFloatBuffer(GLuint buffer, int n){
    m_resetFloatBufferShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer);
    glUniform1i(ShaderProgram::getVarLoc("n"), n);
    m_resetFloatBufferShader.dispatch((n + 255) / 256);
}

void FlipSolverGPU::particlesToGrid(){

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_partPosBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_partVelBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_rXBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_rYBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_rZBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_velXBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, m_velYBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, m_velZBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, m_isAirBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, m_firstCellParticleBuffer);

    m_p2gShader.use();
    glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config->getGridX());
    glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config->getGridY());
    glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config->getGridZ());
    glUniform1f(ShaderProgram::getVarLoc("h"), m_config->getH());
    glUniform1ui(ShaderProgram::getVarLoc("isFull"), m_isFull ? 1u : 0u);

    glUniform2i(ShaderProgram::getVarLoc("openBoundsX"), m_config->getOpenBoundsX().x ? 1 : 0, m_config->getOpenBoundsX().y ? 1 : 0);
    glUniform2i(ShaderProgram::getVarLoc("openBoundsY"), m_config->getOpenBoundsY().x ? 1 : 0, m_config->getOpenBoundsY().y ? 1 : 0);
    glUniform2i(ShaderProgram::getVarLoc("openBoundsZ"), m_config->getOpenBoundsZ().x ? 1 : 0, m_config->getOpenBoundsZ().y ? 1 : 0);
    m_p2gShader.dispatch((m_config->getGridDim() + 7) / 8);
    
    ShaderProgram::SSBOBarrier();

    m_applyWeightsShader.use();
    glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config->getGridX());
    glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config->getGridY());
    glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config->getGridZ());
    glUniform1f(ShaderProgram::getVarLoc("h"), m_config->getH());
    m_applyWeightsShader.dispatch((m_config->getGridX()+1 + 7) / 8, (m_config->getGridY()+1 + 7) / 8, (m_config->getGridZ()+1 + 7) / 8);

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    glBindBuffer(GL_COPY_READ_BUFFER, m_velXBuffer);
    glBindBuffer(GL_COPY_WRITE_BUFFER, m_oldVelXBuffer);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, (m_config->getGridX() + 1) * m_config->getGridY() * m_config->getGridZ() * sizeof(float));
    glBindBuffer(GL_COPY_READ_BUFFER, m_velYBuffer);
    glBindBuffer(GL_COPY_WRITE_BUFFER, m_oldVelYBuffer);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, m_config->getGridX() * (m_config->getGridY() + 1) * m_config->getGridZ() * sizeof(float));
    glBindBuffer(GL_COPY_READ_BUFFER, m_velZBuffer);
    glBindBuffer(GL_COPY_WRITE_BUFFER, m_oldVelZBuffer);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, m_config->getGridX() * m_config->getGridY() * (m_config->getGridZ() + 1) * sizeof(float));
}

void FlipSolverGPU::surfaceTension(){
    ShaderProgram::SSBOBarrier();

    m_computeRhoShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_rXBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_rYBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_rZBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_isAirBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_rhoBuffer);
    glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config->getGridX());
    glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config->getGridY());
    glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config->getGridZ());
    glUniform1f(ShaderProgram::getVarLoc("h"), m_config->getH());
    m_computeRhoShader.dispatch((m_config->getGridDim() + 7) / 8);

    integrateGrid();
    
    // ShaderProgram::SSBOBarrier();

    // m_smoothDataShader.use();
    // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_rhoBuffer);
    // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_smoothRhoBuffer);
    // glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config->getGridX());
    // glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config->getGridY());
    // glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config->getGridZ());
    // glUniform1f(ShaderProgram::getVarLoc("h"), m_config->getH());
    // m_smoothDataShader.dispatch((m_config->getGridDim() + 7) / 8);
    
    // ShaderProgram::SSBOBarrier();

    // m_computeGradShader.use();
    // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_smoothRhoBuffer);
    // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_gradRhoBuffer);
    // glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config->getGridX());
    // glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config->getGridY());
    // glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config->getGridZ());
    // glUniform1f(ShaderProgram::getVarLoc("h"), m_config->getH());
    // m_computeGradShader.dispatch((m_config->getGridDim() + 7) / 8);
    
    // ShaderProgram::SSBOBarrier();

    // m_computeCurvatureShader.use();
    // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_gradRhoBuffer);
    // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_curvatureBuffer);
    // glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config->getGridX());
    // glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config->getGridY());
    // glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config->getGridZ());
    // glUniform1f(ShaderProgram::getVarLoc("h"), m_config->getH());
    // m_computeCurvatureShader.dispatch((m_config->getGridDim() + 7) / 8);
    
    // ShaderProgram::SSBOBarrier();

    // m_integrateGridShader.use();
    // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_curvatureBuffer);
    // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_smoothRhoBuffer);
    // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_velXBuffer);
    // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_velYBuffer);
    // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_velZBuffer);
    // glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config->getGridX());
    // glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config->getGridY());
    // glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config->getGridZ());
    // glUniform1f(ShaderProgram::getVarLoc("h"), m_config->getH());
    // glUniform1f(ShaderProgram::getVarLoc("dt"), m_config->getDt());
    // glUniform1f(ShaderProgram::getVarLoc("sigma"), m_config->getSigma());
    // m_integrateGridShader.dispatch((m_config->getGridDim() + 7) / 8);
}

void FlipSolverGPU::precomputeCoarseIsAir(bool dispatch){
    ivec3 dim = m_config->getGridDim();

    m_mgCoarseIsAir.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_isAirBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_coarseIsAirBuffers[0]);
    glUniform1i(ShaderProgram::getVarLoc("gridX"), dim.x);
    glUniform1i(ShaderProgram::getVarLoc("gridY"), dim.y);
    glUniform1i(ShaderProgram::getVarLoc("gridZ"), dim.z);
    glUniform1i(ShaderProgram::getVarLoc("coarseGridX"), dim.x);
    glUniform1i(ShaderProgram::getVarLoc("coarseGridY"), dim.y);
    glUniform1i(ShaderProgram::getVarLoc("coarseGridZ"), dim.z);

    if (dispatch){
        m_mgCoarseIsAir.dispatch((dim + 7) / 8);
        ShaderProgram::SSBOBarrier();
    } else {
        ShaderProgram::indirectDispatch(m_indirectBuffer, (m_indirectParamsIndex + 0) * sizeof(DispatchParams));
        ShaderProgram::indirectBarrier();
    }

    for (int l = 0; l < (int)m_zBuffers.size() - 1; l++)
    {
        ivec3 coarseDim = (dim + 1) / 2;
        m_mgCoarseIsAir.use();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_coarseIsAirBuffers[l]);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_coarseIsAirBuffers[l+1]);
        glUniform1i(ShaderProgram::getVarLoc("gridX"), dim.x);
        glUniform1i(ShaderProgram::getVarLoc("gridY"), dim.y);
        glUniform1i(ShaderProgram::getVarLoc("gridZ"), dim.z);
        glUniform1i(ShaderProgram::getVarLoc("coarseGridX"), coarseDim.x);
        glUniform1i(ShaderProgram::getVarLoc("coarseGridY"), coarseDim.y);
        glUniform1i(ShaderProgram::getVarLoc("coarseGridZ"), coarseDim.z);

        if (dispatch) {
            m_mgCoarseIsAir.dispatch((coarseDim + 7) / 8);
            ShaderProgram::SSBOBarrier();
        } else {
            ShaderProgram::indirectDispatch(m_indirectBuffer, (m_indirectParamsIndex + l+1) * sizeof(DispatchParams));
            ShaderProgram::indirectBarrier();
        }

        dim = coarseDim;
    }
}

void FlipSolverGPU::smooth(int l, int color, ivec3 dim, bool dispatch){
    m_mgSmooth.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_rBuffers[l]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_coarseIsAirBuffers[l]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_zBuffers[l]);
    glUniform1i(ShaderProgram::getVarLoc("color"), color);
    glUniform1i(ShaderProgram::getVarLoc("gridX"), dim.x);
    glUniform1i(ShaderProgram::getVarLoc("gridY"), dim.y);
    glUniform1i(ShaderProgram::getVarLoc("gridZ"), dim.z);

    if (dispatch){
        m_mgSmooth.dispatch((dim + 7) / 8);
        ShaderProgram::SSBOBarrier();
    } else {
        ShaderProgram::indirectDispatch(m_indirectBuffer, (m_indirectParamsIndex + l) * sizeof(DispatchParams));
        ShaderProgram::indirectBarrier();
    }
}

void FlipSolverGPU::restrict(int l, ivec3 dim, bool dispatch) {
    ivec3 coarseDim = (dim + 1) / 2;
    m_mgRestrict.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_rBuffers[l]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_rBuffers[l+1]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_coarseIsAirBuffers[l]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_zBuffers[l]);
    glUniform1i(ShaderProgram::getVarLoc("gridX"), dim.x);
    glUniform1i(ShaderProgram::getVarLoc("gridY"), dim.y);
    glUniform1i(ShaderProgram::getVarLoc("gridZ"), dim.z);
    glUniform1i(ShaderProgram::getVarLoc("coarseGridX"), coarseDim.x);
    glUniform1i(ShaderProgram::getVarLoc("coarseGridY"), coarseDim.y);
    glUniform1i(ShaderProgram::getVarLoc("coarseGridZ"), coarseDim.z);

    if (dispatch) {
        m_mgRestrict.dispatch((coarseDim + 7) / 8);
        ShaderProgram::SSBOBarrier();
    } else {
        ShaderProgram::indirectDispatch(m_indirectBuffer, (m_indirectParamsIndex + l+1) * sizeof(DispatchParams));
        ShaderProgram::indirectBarrier();
    }
}

void FlipSolverGPU::prolongate(int l, ivec3 dim, bool dispatch){
    m_mgProlongate.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_zBuffers[l]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_zBuffers[l+1]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_coarseIsAirBuffers[l]);
    glUniform1i(ShaderProgram::getVarLoc("gridX"), dim.x);
    glUniform1i(ShaderProgram::getVarLoc("gridY"), dim.y);
    glUniform1i(ShaderProgram::getVarLoc("gridZ"), dim.z);
    glUniform1i(ShaderProgram::getVarLoc("coarseGridX"), (dim.x + 1) / 2);
    glUniform1i(ShaderProgram::getVarLoc("coarseGridY"), (dim.y + 1) / 2);
    glUniform1i(ShaderProgram::getVarLoc("coarseGridZ"), (dim.z + 1) / 2);

    if (dispatch) {
        m_mgProlongate.dispatch((dim + 7) / 8);
        ShaderProgram::SSBOBarrier();
    } else {
        ShaderProgram::indirectDispatch(m_indirectBuffer, (m_indirectParamsIndex + l) * sizeof(DispatchParams));
        ShaderProgram::indirectBarrier();
    }
}

void FlipSolverGPU::MGPrecond(GLuint zBuffer, GLuint rBuffer, bool dispatch) {
    const int RED = 0;
    const int BLACK = 1;

    ivec3 dim = m_config->getGridDim();
    vector<ivec3> dims = {};

    MatOps* matOps = m_cgs.getMatOps();
    matOps->copy(rBuffer, m_rBuffers[0], dim.x * dim.y * dim.z, dispatch);
    if (!dispatch) {
        ShaderProgram::indirectDispatch(m_indirectBuffer, (m_indirectParamsIndex + (int)m_zBuffers.size() + 0) * sizeof(DispatchParams));
        ShaderProgram::indirectBarrier();
    }

    for (int l = 0; l < (int)m_zBuffers.size() - 1; l++)
    {
        dims.push_back(dim);
        int n = dim.x * dim.y * dim.z;

        matOps->clear(m_zBuffers[l], n, 0.0f, dispatch);
        if (!dispatch) {
            ShaderProgram::indirectDispatch(m_indirectBuffer, (m_indirectParamsIndex + (int)m_zBuffers.size() + l) * sizeof(DispatchParams));
            ShaderProgram::indirectBarrier();
        }

        // Pre-smooth
        for (int nu = 0; nu < 2; nu++){
            smooth(l, RED, dim, dispatch);
            smooth(l, BLACK, dim, dispatch);
        }
        // Restrict
        restrict(l, dim, dispatch);

        dim = (dim + 1) / 2;
    }

    int last = (int)m_zBuffers.size() - 1;
    matOps->clear(m_zBuffers[last], dim.x * dim.y * dim.z, 0.0f, dispatch);
    for (int i = 0; i < 2; i++)
    {
        smooth(last, RED, dim, dispatch);
        smooth(last, BLACK, dim, dispatch);
        smooth(last, BLACK, dim, dispatch);
        smooth(last, RED, dim, dispatch);
    }

    for (int l = (int)m_zBuffers.size() - 2; l >= 0; l--)
    {
        dim = dims[l];
        
        // Prolongate
        prolongate(l, dim, dispatch);

        // Post-smooth
        for (int nu = 0; nu < 2; nu++)
        {
            smooth(l, BLACK, dim, dispatch);
            smooth(l, RED, dim, dispatch);
        }
    }

    matOps->copy(m_zBuffers[0], zBuffer, dim.x * dim.y * dim.z, dispatch);
    if (!dispatch) {
        ShaderProgram::indirectDispatch(m_indirectBuffer, (m_indirectParamsIndex + (int)m_zBuffers.size() + 0) * sizeof(DispatchParams));
        ShaderProgram::indirectBarrier();
    }
}

void FlipSolverGPU::solveIncompressibility(int iterations, float tol, bool useCGS){
    ShaderProgram::SSBOBarrier();

    if (useCGS){
        m_computeMinusDivShader.use();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_velXBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_velYBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_velZBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_rhoBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_isAirBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_minusDivBuffer);
        glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config->getGridX());
        glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config->getGridY());
        glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config->getGridZ());
        glUniform1f(ShaderProgram::getVarLoc("h"), m_config->getH());
        glUniform1f(ShaderProgram::getVarLoc("density"), m_config->getDensity() * m_config->getDensityMultiplier());
        glUniform1f(ShaderProgram::getVarLoc("densityMultiplier"), 1);
        m_computeMinusDivShader.dispatch((m_config->getGridDim() + 7) / 8);
        
        ShaderProgram::SSBOBarrier();
        
        m_setAirCellsToZeroShader.use();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_isAirBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_pressureBuffer);
        glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config->getGridX());
        glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config->getGridY());
        glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config->getGridZ());
        glUniform1f(ShaderProgram::getVarLoc("h"), m_config->getH());
        m_setAirCellsToZeroShader.dispatch((m_config->getGridDim() + 7) / 8);
    
        ShaderProgram::SSBOBarrier();
    
        precomputeCoarseIsAir();

        DispatchParams sparseMatVecParams = { 
            (uint)(m_config->getGridX() + 7) / 8, 
            (uint)(m_config->getGridY() + 7) / 8, 
            (uint)(m_config->getGridZ() + 7) / 8 
        };
        m_cgs.solve(iterations, tol, sparseMatVecParams, true, m_isFull);
    
        ShaderProgram::SSBOBarrier();
    
        m_pressureToVelShader.use();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_pressureBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_isAirBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_velXBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_velYBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_velZBuffer);
        glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config->getGridX());
        glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config->getGridY());
        glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config->getGridZ());
        glUniform1f(ShaderProgram::getVarLoc("h"), m_config->getH());
        m_pressureToVelShader.dispatch((m_config->getGridDim() + 7) / 8);
    
        ShaderProgram::SSBOBarrier();

        m_computeMinusDivShader.use();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_velXBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_velYBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_velZBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_rhoBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_isAirBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_minusDivBuffer);
        glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config->getGridX());
        glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config->getGridY());
        glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config->getGridZ());
        glUniform1f(ShaderProgram::getVarLoc("h"), m_config->getH());
        m_computeMinusDivShader.dispatch((m_config->getGridDim() + 7) / 8);
    }
    else{
        m_solveIncompressibilityShader.use();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_rXBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_rYBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_rZBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_velXBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_velYBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_velZBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, m_isAirBuffer);
        glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config->getGridX());
        glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config->getGridY());
        glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config->getGridZ());
        glUniform1f(ShaderProgram::getVarLoc("h"), m_config->getH());
    
        int isPair = 0;
    
        for (int i = 0; i < 2 * iterations; i++)
        {
            ShaderProgram::SSBOBarrier();
            
            m_solveIncompressibilityShader.use();
            glUniform1i(ShaderProgram::getVarLoc("isPair"), isPair);
            m_solveIncompressibilityShader.dispatch((m_config->getGridDim() + 7) / 8);
    
            isPair = 1 - isPair;
        }
    }
}

void FlipSolverGPU::gridToParticles(){
    ShaderProgram::SSBOBarrier();

    m_g2pShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_partPosBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_partVelBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_velXBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_velYBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_velZBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_oldVelXBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, m_oldVelYBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, m_oldVelZBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, m_isAirBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, m_aliveNBuffer);
    glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config->getGridX());
    glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config->getGridY());
    glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config->getGridZ());
    glUniform1f(ShaderProgram::getVarLoc("h"), m_config->getH());
    glUniform1f(ShaderProgram::getVarLoc("flipRatio"), m_config->getFlipRatio());
    m_g2pShader.dispatch((m_config->getPartN() + 63) / 64);
}

void FlipSolverGPU::update(){
    if (m_isPaused) return;

    auto lockedObstacle = m_obstacle.lock();

    m_integrateTimer.beginFrame();
    integrateParticles();
    m_integrateTimer.endFrame();

    particleCollisions(lockedObstacle);
    
    m_pushAppartTimer.beginFrame();
    pushAppartParticles(m_config->getNumPushAppart());
    m_pushAppartTimer.endFrame();

    m_collisionTimer.beginFrame();
    particleCollisions(lockedObstacle);
    m_collisionTimer.endFrame();

    m_scanTimer.beginFrame();
    countingSort();
    ShaderProgram::SSBOBarrier();
    m_scanTimer.endFrame();
    
    m_p2gTimer.beginFrame();
    particlesToGrid();
    m_p2gTimer.endFrame();

    m_surfaceTensionTimer.beginFrame();
    surfaceTension();
    m_surfaceTensionTimer.endFrame();

    m_incompressibilityTimer.beginFrame();
    solveIncompressibility(m_config->getCgMaxIter(), m_config->getCgTol(), true);
    m_incompressibilityTimer.endFrame();

    m_g2pTimer.beginFrame();
    gridToParticles();
    m_g2pTimer.endFrame();
    
    m_stats->setTimer(m_integrationStatIndex,       m_integrateTimer.getLastResultMs());
    m_stats->setTimer(m_collisionStatIndex,         2 * m_collisionTimer.getLastResultMs());
    m_stats->setTimer(m_pushAppartStatIndex,        m_pushAppartTimer.getLastResultMs());
    m_stats->setTimer(m_p2gStatIndex,               m_p2gTimer.getLastResultMs());
    m_stats->setTimer(m_surfaceTensionStatIndex,    m_surfaceTensionTimer.getLastResultMs());
    m_stats->setTimer(m_incompressibilityStatIndex, m_incompressibilityTimer.getLastResultMs());
    m_stats->setTimer(m_g2pStatIndex,               m_g2pTimer.getLastResultMs());
    m_stats->setTimer(m_scanStatIndex,              m_scanTimer.getLastResultMs());
}

void FlipSolverGPU::attributeToGrid(GLuint attributesBuffer, GLuint gridTex, GLuint weightsBuffer){
    m_a2gShader.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_partPosBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, attributesBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_isAirBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_firstCellParticleBuffer);
    glBindImageTexture(0, gridTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
    glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config->getGridX());
    glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config->getGridY());
    glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config->getGridZ());
    glUniform1f(ShaderProgram::getVarLoc("h"), m_config->getH());
    m_a2gShader.dispatch((m_config->getGridDim() + 7) / 8);
}