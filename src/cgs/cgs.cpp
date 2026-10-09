#include "cgs.hpp"

using namespace std;

CGS::CGS() {}
CGS::~CGS() {
    deleteBuffers();
}

void CGS::deleteBuffers(){
    glDeleteBuffers(1, &m_ATABuffer);
    glDeleteBuffers(1, &m_ATBuffer);
    glDeleteBuffers(1, &m_ATbBuffer);
    glDeleteBuffers(1, &m_zeroBuffer);
    glDeleteBuffers(1, &m_zBuffer);
    glDeleteBuffers(1, &m_dBuffer);
    glDeleteBuffers(1, &m_AdBuffer);
    glDeleteBuffers(1, &m_rBuffer);
    glDeleteBuffers(1, &m_scalarBuffer);
    glDeleteBuffers(1, &m_indirectBuffer);
}

void CGS::initBuffers(){
    glGenBuffers(1, &m_ATABuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ATABuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_n * m_n * sizeof(float), nullptr, GL_DYNAMIC_COPY);
    
    glGenBuffers(1, &m_ATBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ATBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_n * m_n * sizeof(float), nullptr, GL_DYNAMIC_COPY);
    
    glGenBuffers(1, &m_ATbBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ATbBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_n * sizeof(float), nullptr, GL_DYNAMIC_COPY);

    glGenBuffers(1, &m_zeroBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_zeroBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_n * sizeof(float), vector<float>(m_n, 0.0f).data(), GL_DYNAMIC_COPY);

    glGenBuffers(1, &m_zBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_zBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_n * sizeof(float), vector<float>(m_n, 0.0f).data(), GL_DYNAMIC_COPY);

    glGenBuffers(1, &m_dBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_dBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_n * sizeof(float), vector<float>(m_n, 0.0f).data(), GL_DYNAMIC_COPY);
    
    glGenBuffers(1, &m_AdBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_AdBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_n * sizeof(float), vector<float>(m_n, 0.0f).data(), GL_DYNAMIC_COPY);

    glGenBuffers(1, &m_rBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_rBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_n * sizeof(float), vector<float>(m_n, 0.0f).data(), GL_DYNAMIC_COPY);
    
    vector<float> scalars = vector<float>(NUM_SCALARS, 0);
    scalars[MINUS_ONE_SCALAR_INDEX] = -1;
    glGenBuffers(1, &m_scalarBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_scalarBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_SCALARS * sizeof(float), scalars.data(), GL_DYNAMIC_COPY);

    m_dispatchParams = vector<DispatchParams>{
        {GLuint((m_n + 63) / 64), 1, 1},     // matVec
        {GLuint((m_n + 127) / 128), 1, 1},   // dot1
        {GLuint((m_n + 255) / 256), 1, 1},   // saxpy
        {1, 1, 1},                         // constant
    };

    glGenBuffers(1, &m_indirectBuffer);
    glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, m_indirectBuffer);
    glBufferData(GL_DISPATCH_INDIRECT_BUFFER, m_dispatchParams.size() * sizeof(DispatchParams), 
                                              m_dispatchParams.data(), GL_DYNAMIC_DRAW);
}

void CGS::compute_ATA_ATb(GLuint AMatrixbuffer, GLuint bMatrixBuffer){
    m_matOps->transpose(AMatrixbuffer, m_ATBuffer, m_n);
    m_matOps->multiply(m_ATBuffer, AMatrixbuffer, m_ATABuffer, m_n);
    m_matOps->copy(m_ATABuffer, AMatrixbuffer, m_n * m_n);

    m_matOps->matVec(m_ATBuffer, bMatrixBuffer, m_ATbBuffer, m_n);
    m_matOps->copy(m_ATbBuffer, bMatrixBuffer, m_n);
}

void CGS::init(int n, GLuint ABuffer, GLuint bBuffer, GLuint xBuffer){
    this->m_n = n;
    deleteBuffers();
    initBuffers();
    
    m_stopProg.destroy();
    m_stopProg.create();
    m_stopProg.load(GL_COMPUTE_SHADER, "src/shaders/mat/cgs_stop_cond.glsl");
    m_stopProg.link();
    
    m_matOps = make_unique<MatOps>();
    
    this->m_ABuffer = ABuffer;
    this->m_bBuffer = bBuffer;
    this->m_xBuffer = xBuffer;
}

void CGS::reloadArgs(GLuint ABuffer, GLuint bBuffer, GLuint xBuffer){
    this->m_ABuffer = ABuffer;
    this->m_bBuffer = bBuffer;
    this->m_xBuffer = xBuffer;
}

void CGS::swap_rtr_indices(){
    int temp = RTR_NEW_SCALAR_INDEX;
    RTR_NEW_SCALAR_INDEX = RTR_SCALAR_INDEX;
    RTR_SCALAR_INDEX = temp;
}

void CGS::computeZ() {

}

GLuint CGS::solve(int maxIter, float tol){
    std::function<void(GLuint, GLuint, GLuint, int, bool)> matVec =
    [this](GLuint Abuf, GLuint dBuf, GLuint AdBuf, int n, bool dispatch) {
        m_matOps->matVec(Abuf, dBuf, AdBuf, n, dispatch);
    };
    return solve(maxIter, tol, matVec, m_dispatchParams[0], false);
}

GLuint CGS::solve(int maxIter, float tol, function<void(GLuint, GLuint, GLuint, int, bool)> matVec, DispatchParams matVecParams, bool reuse){
    m_dispatchParams[0] = matVecParams;
    glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, m_indirectBuffer);
    glBufferData(GL_DISPATCH_INDIRECT_BUFFER, m_dispatchParams.size() * sizeof(DispatchParams), 
                                              m_dispatchParams.data(), GL_DYNAMIC_DRAW);
    
    static vector<int> xAlphaScalarOperations = vector<int>(16,0);
    static vector<int> rAlphaScalarOperations = vector<int>(16,0);
    static vector<int> betaScalarOperations = vector<int>(16,0);
    static vector<int> minusOperations = vector<int>(16,0);

    // saxpy exponent operators
    minusOperations[MINUS_ONE_SCALAR_INDEX] = 1;
    xAlphaScalarOperations[RTR_NEW_SCALAR_INDEX] = 1;
    xAlphaScalarOperations[DTAD_SCALAR_INDEX] = -1;
    rAlphaScalarOperations[RTR_NEW_SCALAR_INDEX] = 1;
    rAlphaScalarOperations[DTAD_SCALAR_INDEX] = -1;
    rAlphaScalarOperations[MINUS_ONE_SCALAR_INDEX] = 1;
    betaScalarOperations[RTR_NEW_SCALAR_INDEX] = 1;
    betaScalarOperations[RTR_SCALAR_INDEX] = -1;
    
    vector<float> initScalars = vector<float>(NUM_SCALARS, 0.0f);
    initScalars[MINUS_ONE_SCALAR_INDEX] = -1.0f;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_scalarBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, initScalars.size() * sizeof(float), initScalars.data(), GL_DYNAMIC_DRAW);
    
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    
    // clear x
    if (!reuse) m_matOps->copy(m_zeroBuffer, m_xBuffer, m_n);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    // Ad = A * x
    matVec(m_ABuffer, m_xBuffer, m_AdBuffer, m_n, true);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    // r = b - Ad
    m_matOps->saxpy(m_bBuffer, m_AdBuffer, m_rBuffer, m_scalarBuffer, m_n, minusOperations, true);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    computeZ();        
    
    // d = r
    m_matOps->copy(m_rBuffer, m_dBuffer, m_n);
    // compute rTrnew
    m_matOps->dot(m_rBuffer, m_rBuffer, m_scalarBuffer, m_n, glm::ivec3(RTR_NEW_SCALAR_INDEX, RTR0_VALUE_INDEX, RTR_SCALAR_INDEX));
    for (int i = 0; i < maxIter; i++)
    {
        // Compute Ad
        ShaderProgram::indirectBarrier();
        matVec(m_ABuffer, m_dBuffer, m_AdBuffer, m_n, false);
        ShaderProgram::indirectDispatch(m_indirectBuffer, 0 * sizeof(DispatchParams));

        // Compute dTAd
        ShaderProgram::indirectBarrier();
        m_matOps->dotIndirect(
            m_dBuffer, 
            m_AdBuffer, 
            m_scalarBuffer, 
            m_indirectBuffer, 
            1 * sizeof(DispatchParams),
            3 * sizeof(DispatchParams),
            m_n, 
            DTAD_SCALAR_INDEX
        );
        //ShaderProgram::indirectDispatch(indirectBuffer, 1 * sizeof(DispatchParams));

        // alpha = rTrnew / dTad
        // Compute x += alpha * d
        ShaderProgram::indirectBarrier();
        m_matOps->saxpy(m_xBuffer, m_dBuffer, m_xBuffer, m_scalarBuffer, m_n, xAlphaScalarOperations, false);
        ShaderProgram::indirectDispatch(m_indirectBuffer, 2 * sizeof(DispatchParams));

        // Compute r -= alpha * Ad
        m_matOps->saxpy(m_rBuffer, m_AdBuffer, m_rBuffer, m_scalarBuffer, m_n, rAlphaScalarOperations, false);
        ShaderProgram::indirectDispatch(m_indirectBuffer, 2 * sizeof(DispatchParams));

        // rTr<->rTrnew, Compute rTrnew 
        swap_rtr_indices();
        ShaderProgram::indirectBarrier();
        m_matOps->dotIndirect(
            m_rBuffer, 
            m_rBuffer, 
            m_scalarBuffer, 
            m_indirectBuffer,
            1 * sizeof(DispatchParams),
            3 * sizeof(DispatchParams),
            m_n, 
            RTR_NEW_SCALAR_INDEX
        );
        // m_matOps->dot(m_rBuffer, m_rBuffer, m_scalarBuffer, m_n, RTR_NEW_SCALAR_INDEX);
        // ShaderProgram::indirectDispatch(indirectBuffer, 1 * sizeof(DispatchParams));
        xAlphaScalarOperations = vector<int>(16,0);
        xAlphaScalarOperations[RTR_NEW_SCALAR_INDEX] = 1;
        xAlphaScalarOperations[DTAD_SCALAR_INDEX] = -1;
        rAlphaScalarOperations = vector<int>(16,0);
        rAlphaScalarOperations[RTR_NEW_SCALAR_INDEX] = 1;
        rAlphaScalarOperations[DTAD_SCALAR_INDEX] = -1;
        rAlphaScalarOperations[MINUS_ONE_SCALAR_INDEX] = 1;
        betaScalarOperations = vector<int>(16,0);
        betaScalarOperations[RTR_NEW_SCALAR_INDEX] = 1;
        betaScalarOperations[RTR_SCALAR_INDEX] = -1;
        
        // Compute d = r + rTrnew / rTr * d
        ShaderProgram::indirectBarrier();
        m_matOps->saxpy(m_rBuffer, m_dBuffer, m_dBuffer, m_scalarBuffer, m_n, betaScalarOperations, false);
        ShaderProgram::indirectDispatch(m_indirectBuffer, 2 * sizeof(DispatchParams));
        

        // Stop condition + clean scalar
        m_stopProg.use();
        glUniform1i(ShaderProgram::getVarLoc("rTrIndex"), RTR_NEW_SCALAR_INDEX);
        glUniform1i(ShaderProgram::getVarLoc("rTr0ValueIndex"), RTR0_VALUE_INDEX);
        glUniform1i(ShaderProgram::getVarLoc("numScalars"), NUM_SCALARS);
        glUniform1i(ShaderProgram::getVarLoc("numIndirectParams"), (int)m_dispatchParams.size());
        glUniform1f(ShaderProgram::getVarLoc("tol"), tol);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_scalarBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_indirectBuffer);
        ShaderProgram::indirectDispatch(m_indirectBuffer, 3 * sizeof(DispatchParams));
    }

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    
    return m_xBuffer;
}