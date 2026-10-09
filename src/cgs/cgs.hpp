#ifndef CGS_HPP
#define CGS_HPP

#include <vector>
#include <functional>
#include <glad/glad.h>
#include <memory>

#include "matrix_ops.hpp"
#include "core/shader_program.hpp"

class CGS {
private:
    int RTR_SCALAR_INDEX = 0;
    int RTR_NEW_SCALAR_INDEX = 1;
    static constexpr int DTAD_SCALAR_INDEX = 2;
    static constexpr int RTR0_VALUE_INDEX = 3;
    static constexpr int MINUS_ONE_SCALAR_INDEX = 9;
    static constexpr int NUM_SCALARS = 10;

    int m_n = 0;
    GLuint m_ABuffer = 0;
    GLuint m_bBuffer = 0;
    GLuint m_ATBuffer = 0;
    GLuint m_ATbBuffer = 0;
    GLuint m_ATABuffer = 0;
    
    GLuint m_zeroBuffer = 0;
    GLuint m_xBuffer = 0;
    GLuint m_zBuffer = 0;
    GLuint m_dBuffer = 0;
    GLuint m_AdBuffer = 0;
    GLuint m_rBuffer = 0;
    GLuint m_scalarBuffer = 0;
    GLuint m_indirectBuffer = 0;
    std::vector<DispatchParams> m_dispatchParams = {};
    ShaderProgram m_stopProg = {};

    std::unique_ptr<MatOps> m_matOps;

    void initBuffers();
    void deleteBuffers();
    void swap_rtr_indices();
    void computeZ();

public:
    CGS();
    ~CGS();

    CGS(const CGS&) = delete;
    CGS& operator=(const CGS&) = delete;

    void compute_ATA_ATb(GLuint AMatrixbuffer, GLuint bMatrixBuffer);
    /**
     * @brief Initialize the solver
     * @param n         Size of the matrices
     * @param Abuffer   n*n square matrix
     * @param bBuffer   n vector
     * @param xBuffer   n vector, will contain the solution of Ax = b when the system is solved
     */
    void init(int n, GLuint ABuffer, GLuint bBuffer, GLuint xBuffer);
    /**
     * @brief Reload the solver
     * @param Abuffer   n*n square matrix
     * @param bBuffer   n vector
     * @param xBuffer   n vector, will contain the solution of Ax = b when the system is solved
     */
    void reloadArgs(GLuint ABuffer, GLuint bBuffer, GLuint xBuffer);
    /**
     * @brief Solve Ax = b. A x and b must be passed through \ref init(n, Abuffer, bBuffer, xBuffer)
     * @param maxIter   Maximum number of iteration to solve the system
     * @param tol       Tolerance of the system. The solver stops when the error has decreased by a multiplication of \p tol
     */
    GLuint solve(int maxIter, float tol);
    /**
     * @brief Solve Ax = b. A x and b must be passed through \ref init(n, Abuffer, bBuffer, xBuffer)
     * @note \p matVec can be used to implement a matrix free operation for sparse matrices in some cases
     * @param maxIter       Maximum number of iteration to solve the system
     * @param tol           Tolerance of the system. The solver stops when the error has decreased by a multiplication of \p tol
     * @param matVec        Custom matrix-vector multiplication function. Its arguments are:
     * 
     *                      - ABuffer : The matrix A of the system
     * 
     *                      - dBuffer : The matrix d, which is the step of the solver
     * 
     *                      - AdBuffer : The buffer in which to store the result of A * d
     * 
     *                      - n : The size of the vectors
     * 
     *                      - dispatch : whether or not \p matVec should dispatch the compute shaders. This is used when the solver does some indirect dispatching.
     * @param matVecParams  The dispatch parameters of the compute shader used in matVec, used for some indirect dispatching.
     * @param reuse         Default is true. Describes whether or not the value of \p xBuffer is used as the initialization for the solver.
     */
    GLuint solve(int maxIter, float tol, std::function<void(GLuint, GLuint, GLuint, int, bool)> matVec, DispatchParams matVecParams, bool reuse=true);
};

#endif