#ifndef MAT_OPT_HPP
#define MAT_OPT_HPP

#include <glad/glad.h>
#include "core/shader_program.hpp"
#include <glm/glm.hpp>

struct DispatchParams {
    GLuint num_groups_x;
    GLuint num_groups_y;
    GLuint num_groups_z;
};

class MatOps {
private:
    GLuint m_alphaBuffer = 0;
    GLuint m_partialDotBuffer = 0;

    int m_partialDotCapacity = 0;
    
    ShaderProgram m_mulProg = {};
    ShaderProgram m_transposeProg = {};
    ShaderProgram m_matVecProg = {};
    ShaderProgram m_saxpyProg = {};
    ShaderProgram m_dotPartialMultProg = {};
    ShaderProgram m_dotFinalSumProg = {};
    ShaderProgram m_resetScalarProg = {};
    ShaderProgram m_copyProg = {};
    ShaderProgram m_clearProg = {};

    void ensurePartialCapacity(int N);
public:
    MatOps();
    ~MatOps();

    MatOps(const MatOps&) = delete;
    MatOps& operator=(const MatOps&) = delete;

    /**
     * @brief Multiplies two square matrices of size N*N
     * @note Dispatch is often used in cases where you would want to do some indirect dispatching 
     * @param bufferA       Left N*N matrix
     * @param bufferB       Right N*N matrix
     * @param bufferResult  Where the N*N result is stored
     * @param N             Size of the N*N matrices
     * @param dispatch      When set to false, the dispatch command and the barrier command are not executed
     */
    void multiply(GLuint bufferA, GLuint bufferB, GLuint bufferResult, int N, bool dispatch = true);
    
    /**
     * @brief Computes the dot product of two matrices
     * @param bufferA       Left N matrix
     * @param bufferB       Right N matrix
     * @param bufferResult  Where the scalar result is stored
     * @param N             Size of the N matrices
     * @param index         Where in bufferResult to store the result if it is an array.
     */
    void dot(GLuint bufferA, GLuint bufferB, GLuint bufferResult, int N, int index = 0);
    /**
     * @brief Computes the dot product of two matrices
     * @param bufferA       Left N matrix
     * @param bufferB       Right N matrix
     * @param bufferResult  Where the scalar result is stored
     * @param N             Size of the N matrices
     * @param indicies      Where in bufferResult to store the result if it is an array. You can store the result in up to 3 locations in the array
     */
    void dot(GLuint bufferA, GLuint bufferB, GLuint bufferResult, int N, glm::ivec3 indicies);
    /**
     * @brief Computes the dot product of two matrices with indirect dispatching
     * @param bufferA           Left N matrix
     * @param bufferB           Right N matrix
     * @param bufferResult      Where the scalar result is stored
     * @param indirectBuffer    The buffer in which are stored the dispatch parameters. Dispatch parameters should be triplets of uints
     * @param offset1           Offset of the first dispatch parameters. When active, the parameter should be { (N+127)/128, 1, 1 }
     * @param offset2           Offset of the second dispatch parameters. When active, the parameter should be { 1, 1, 1 }
     * @param N                 Size of the N matrices
     * @param index         Where in bufferResult to store the result if it is an array.
     */
    void dotIndirect(GLuint bufferA, GLuint bufferB, GLuint bufferResult, GLuint indirectBuffer, int offset1, int offset2, int N, int index = 0);
    /**
     * @brief Computes Z = X + alpha * Y, with alpha = pow(a1, b1) * pow(a2, b2) * ...
     * @note If ai = 0 and bi = 0, pow(ai, bi) = 1
     * @note Dispatch is often used in cases where you would want to do some indirect dispatching
     * @param bufferX           X matrix
     * @param bufferY           Y matrix
     * @param bufferZ           Z matrix
     * @param bufferAlpha       Buffer containing the (ai)
     * @param alphaOperations   Vector containing the (bi)
     * @param dispatch      When set to false, the dispatch command and the barrier command are not executed
     */
    void saxpy(GLuint bufferX, GLuint bufferY, GLuint bufferZ, 
               GLuint bufferAlpha, int N, std::vector<int> alphaOperations, bool dispatch = true);
    /**
     * @brief Computes Z = X + \p alpha * Y
     * @note Dispatch is often used in cases where you would want to do some indirect dispatching
     * @param bufferX       X matrix
     * @param bufferY       Y matrix
     * @param bufferZ       Z matrix
     * @param alpha         Scalar used for X + alpha * Y
     * @param N             Size of the matrices
     * @param dispatch      When set to false, the dispatch command and the barrier command are not executed
     */
    void saxpy(GLuint bufferX, GLuint bufferY, GLuint bufferZ, float alpha, int N, bool dispatch = true);
    /**
     * @brief Copies \p bufferX into \p bufferY 
     * @note Dispatch is often used in cases where you would want to do some indirect dispatching
     * @param bufferX       Source matrix
     * @param bufferY       Destination matrix
     * @param N             Size of the matrices
     * @param dispatch      When set to false, the dispatch command and the barrier command are not executed
     */
    void copy(GLuint bufferX, GLuint bufferY, int N, bool dispatch = true);
    /**
     * @brief Computes the multiplication of a N*N matrix and a N vector 
     * @note Dispatch is often used in cases where you would want to do some indirect dispatching
     * @param bufferA       N*N matrix
     * @param bufferV       N matrix
     * @param bufferResult  Where the result of size N is stored
     * @param N             Size of the matrices
     * @param dispatch      When set to false, the dispatch command and the barrier command are not executed
     */
    void matVec(GLuint bufferA, GLuint bufferV, GLuint bufferResult, int N, bool dispatch = true);
    /**
     * @brief Transposes a N*N matrix 
     * @note Dispatch is often used in cases where you would want to do some indirect dispatching
     * @param bufferA       N*N matrix
     * @param bufferResult  Where the result is stored
     * @param N             Size of the matrix
     * @param dispatch      When set to false, the dispatch command and the barrier command are not executed
     */
    void transpose(GLuint bufferA, GLuint bufferResult, int N, bool dispatch = true);

    /**
     * @brief Clears the vector with \p value 
     * @note Dispatch is often used in cases where you would want to do some indirect dispatching
     * @param buffer        The vector to clear
     * @param N             The size of the vector
     * @param value         The value each component of the vector will have
     * @param dispatch      When set to false, the dispatch command and the barrier command are not executed
     */
    void clear(GLuint buffer, int N, float value, bool dispatch = true);
};

#endif