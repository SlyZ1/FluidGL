#include "eulerSolverGPU.hpp"

using namespace glm;
using namespace std;

void EulerSolverGPU::deleteBuffers(){
}

void EulerSolverGPU::createBuffers(){
    deleteBuffers();

    FluidInitializer initializer = m_config.getFluidInitializer();

    
}

void EulerSolverGPU::initCGS(){
    m_cgs.init(m_config.getGridX() * m_config.getGridY() * m_config.getGridZ(), 0, m_minusDivBuffer, m_pressureBuffer);

    // std::function<void(GLuint, GLuint, GLuint, int, bool)> sparseMatVec =
    // [this](GLuint, GLuint dBuf, GLuint AdBuf, int, bool dispatch) {
    //     m_sparseMatVecShader.use();
    //     glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, dBuf);
    //     glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_isAirBuffer);
    //     glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, AdBuf);
    //     glUniform1i(ShaderProgram::getVarLoc("gridX"), m_config.getGridX());
    //     glUniform1i(ShaderProgram::getVarLoc("gridY"), m_config.getGridY());
    //     glUniform1i(ShaderProgram::getVarLoc("gridZ"), m_config.getGridZ());
    //     glUniform1f(ShaderProgram::getVarLoc("h"), m_config.getH());
    //     if (dispatch){
    //         m_sparseMatVecShader.dispatch((m_config.getGridDim() + 7) / 8);
    //     }
    // };
    // m_cgs.setCustomMatVec(sparseMatVec);
    m_cgs.setAccuracyFirst(m_config.getAccuracyFirst());
    m_cgs.setPrecond([this](GLuint zBuffer, GLuint rBuffer, bool dispatch) {
        MGPrecond(zBuffer, rBuffer, dispatch);
    });

    uvec3 dim = m_config.getGridDim();
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

EulerSolverGPU::EulerSolverGPU(EulerSolverGPUConfig config) 
: ISolver(make_shared<EulerSolverGPUConfig>(move(config)), "Euler GPU Solver"), m_config(static_cast<EulerSolverGPUConfig&>(*m_baseConfig)) {
    m_surfaceTensionStatIndex     = m_stats->registerTimer("Surface Tension");
    m_incompressibilityStatIndex  = m_stats->registerTimer("Incompressibility");

    m_numCellsStatIndex = m_stats->registerCounter("Num Cells");

    m_surfaceTensionTimer.init();
    m_incompressibilityTimer.init();

    createBuffers();

    initCGS();
}

EulerSolverGPU::~EulerSolverGPU() {
    deleteBuffers();
}

void EulerSolverGPU::reload() {
    m_config = static_cast<EulerSolverGPUConfig&>(*m_baseConfig);
    
    createBuffers();
    
    initCGS();

    ISolver::reload();
}