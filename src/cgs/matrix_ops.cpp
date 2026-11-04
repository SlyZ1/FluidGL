#include "matrix_ops.hpp"

using namespace std;
using namespace glm;

MatOps::MatOps(){
    m_mulProg.create();
    m_mulProg.load(GL_COMPUTE_SHADER, "src/shaders/mat/multiply.glsl");
    m_mulProg.link();
    
    m_transposeProg.create();
    m_transposeProg.load(GL_COMPUTE_SHADER, "src/shaders/mat/transpose.glsl");
    m_transposeProg.link();
    
    m_matVecProg.create();
    m_matVecProg.load(GL_COMPUTE_SHADER, "src/shaders/mat/mat_vec.glsl");
    m_matVecProg.link();
    
    m_saxpyProg.create();
    m_saxpyProg.load(GL_COMPUTE_SHADER, "src/shaders/mat/saxpy.glsl");
    m_saxpyProg.link();
    
    m_dotPartialMultProg.create();
    m_dotPartialMultProg.load(GL_COMPUTE_SHADER, "src/shaders/mat/dotPartialMult.glsl");
    m_dotPartialMultProg.link();
    
    m_dotFinalSumProg.create();
    m_dotFinalSumProg.load(GL_COMPUTE_SHADER, "src/shaders/mat/dotFinalSum.glsl");
    m_dotFinalSumProg.link();

    m_copyProg.create();
    m_copyProg.load(GL_COMPUTE_SHADER, "src/shaders/mat/copy.glsl");
    m_copyProg.link();

    m_resetScalarProg.create();
    m_resetScalarProg.load(GL_COMPUTE_SHADER, "src/shaders/mat/reset_scalar.glsl");
    m_resetScalarProg.link();

    m_clearProg.create();
    m_clearProg.load(GL_COMPUTE_SHADER, "src/shaders/mat/clear.glsl");
    m_clearProg.link();

    glDeleteBuffers(1, &m_partialDotBuffer);
    glGenBuffers(1, &m_partialDotBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_partialDotBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 1024 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    m_partialDotCapacity = 1024;
}

MatOps::~MatOps(){
    glDeleteBuffers(1, &m_partialDotBuffer);
    glDeleteBuffers(1, &m_alphaBuffer);
}

void MatOps::ensurePartialCapacity(int N){
    int needed = (N + 127) / 128;
    if (needed <= m_partialDotCapacity) return;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_partialDotBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, needed * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    m_partialDotCapacity = needed;
}

void MatOps::multiply(GLuint bufferA, GLuint bufferB, GLuint bufferResult, int N, bool dispatch){
    m_mulProg.use();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufferA);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufferB);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, bufferResult);
    glUniform1i(ShaderProgram::getVarLoc("N"), N);
    
    if (!dispatch) return;
    m_mulProg.dispatch((N + 15) / 16, (N + 15) / 16);
    ShaderProgram::SSBOBarrier();
}

void MatOps::dot(GLuint bufferA, GLuint bufferB, GLuint bufferResult, int N, int index){
    dot(bufferA, bufferB, bufferResult, N, ivec3(index, -1, -1));
}

void MatOps::dot(GLuint bufferA, GLuint bufferB, GLuint bufferResult, int N, ivec3 indicies){
    ensurePartialCapacity(N);

    m_dotPartialMultProg.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufferA);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufferB);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_partialDotBuffer);
    glUniform1i(ShaderProgram::getVarLoc("N"), N);
    m_dotPartialMultProg.dispatch((N + 127) / 128);
    
    ShaderProgram::SSBOBarrier();

    m_dotFinalSumProg.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_partialDotBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufferResult);
    glUniform1i(ShaderProgram::getVarLoc("numPartials"), (N + 127) / 128);
    glUniform3i(ShaderProgram::getVarLoc("indicies"), indicies.x, indicies.y, indicies.z);
    m_dotFinalSumProg.dispatch();

    ShaderProgram::SSBOBarrier();
}


void MatOps::dotIndirect(GLuint bufferA, GLuint bufferB, GLuint bufferResult, GLuint indirectBuffer, int offset1, int offset2, int N, int index){
    ensurePartialCapacity(N);

    m_dotPartialMultProg.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufferA);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufferB);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_partialDotBuffer);
    glUniform1i(ShaderProgram::getVarLoc("N"), N);
    
    ShaderProgram::indirectDispatch(indirectBuffer, offset1);
    ShaderProgram::indirectBarrier();

    m_dotFinalSumProg.use();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_partialDotBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufferResult);
    glUniform1i(ShaderProgram::getVarLoc("numPartials"), (N + 127) / 128);
    glUniform3i(ShaderProgram::getVarLoc("indicies"), index, -1, -1);
    
    ShaderProgram::indirectDispatch(indirectBuffer, offset2);
}


void MatOps::saxpy(GLuint bufferX, GLuint bufferY, GLuint bufferZ, 
                   GLuint bufferAlpha, int N, vector<int> alphaOperations, bool dispatch){
    m_saxpyProg.use();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufferX);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufferY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, bufferZ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, bufferAlpha);
    glUniform1i(ShaderProgram::getVarLoc("N"), N);
    glUniform1iv(ShaderProgram::getVarLoc("alpha_operations"), 16, alphaOperations.data());
    
    if (!dispatch) return;
    m_saxpyProg.dispatch((N + 127) / 128);
    ShaderProgram::SSBOBarrier();
}

void MatOps::saxpy(GLuint bufferX, GLuint bufferY, GLuint bufferZ, float alpha, int N, bool dispatch){
    m_saxpyProg.use();

    if (m_alphaBuffer == 0){
        glGenBuffers(1, &m_alphaBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_alphaBuffer);
    }
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float), &alpha, GL_DYNAMIC_COPY);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufferX);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufferY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, bufferZ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_alphaBuffer);
    glUniform1i(ShaderProgram::getVarLoc("N"), N);
    vector<int> alphaOperations(16, 0);
    alphaOperations[0] = 1;
    glUniform1iv(ShaderProgram::getVarLoc("alpha_operations"), 16, alphaOperations.data());
    
    if (!dispatch) return;
    m_saxpyProg.dispatch((N + 127) / 128);
    ShaderProgram::SSBOBarrier();
}

void MatOps::copy(GLuint bufferX, GLuint bufferY, int N, bool dispatch){
    m_copyProg.use();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufferX);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufferY);
    glUniform1i(ShaderProgram::getVarLoc("N"), N);

    if (!dispatch) return;
    m_copyProg.dispatch((N + 255) / 256);
    ShaderProgram::SSBOBarrier();
}

void MatOps::matVec(GLuint bufferA, GLuint bufferV, GLuint bufferResult, int N, bool dispatch){
    m_matVecProg.use();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufferA);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufferV);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, bufferResult);
    glUniform1i(ShaderProgram::getVarLoc("N"), N);
    
    if (!dispatch) return;
    m_matVecProg.dispatch((N + 63) / 64);
    ShaderProgram::SSBOBarrier();
}

void MatOps::transpose(GLuint bufferA, GLuint bufferResult, int N, bool dispatch){
    m_transposeProg.use();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufferA);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufferResult);
    glUniform1i(ShaderProgram::getVarLoc("N"), N);
    
    if (!dispatch) return;
    m_transposeProg.dispatch((N + 15) / 16, (N + 15) / 16);
    ShaderProgram::SSBOBarrier();
}

void MatOps::clear(GLuint buffer, int N, float value, bool dispatch){
    m_clearProg.use();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer);
    glUniform1f(ShaderProgram::getVarLoc("value"), value);
    glUniform1i(ShaderProgram::getVarLoc("N"), N);

    if (!dispatch) return;
    m_clearProg.dispatch((N + 255) / 256);
    ShaderProgram::SSBOBarrier();
}