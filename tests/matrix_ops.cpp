#include <catch2/catch_test_macros.hpp>
#include "src/cgs/matrix_ops.hpp"
#include "src/helpers/logger.hpp"

using namespace std;

static bool testMatrixMultiplication(float eps = 1e-5f){
    MatOps matOps = MatOps();
    GLuint bufferA = 0;
    GLuint bufferB = 0;
    GLuint bufferC = 0;
    
    const int N = 4;
    float a[] = { -1.762f, -3.492f, 1.509f, -4.276f, 0.359f, -1.343f, -4.42f, 0.074f, -4.625f, -0.664f, -4.301f, -4.093f, -0.755f, 3.269f, -3.762f, -2.768f };
    float b[] = { 1.274f, 4.477f, 0.771f, -1.033f, 4.763f, -4.534f, 3.585f, -2.104f, -3.557f, -3.822f, -1.915f, 3.161f, -3.193f, 0.816f, 1.389f, -1.276f };
    float c[] = { -10.591429f, -1.31236f, -22.706421f, 19.393439f, 9.546315f, 24.650029f, 4.02922f, -11.611219f, 19.312724f, -4.597015f, -3.395077f, -2.198112f, 36.828035f, -6.082105f, 14.496738f, -14.457775f };

    glGenBuffers(1, &bufferA); glGenBuffers(1, &bufferB); glGenBuffers(1, &bufferC);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferA);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(a), &a[0], GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferB);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(b), &b[0], GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferC);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(c), nullptr, GL_STATIC_DRAW);

    matOps.multiply(bufferA, bufferB, bufferC, N, true);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferC);
    float result[N * N];
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(c), &result[0]);

    bool isCorrect = true;
    for (int i = 0; i < N * N; i++)
        isCorrect = isCorrect && (abs(c[i] - result[i]) < eps);

    glDeleteBuffers(1, &bufferA); glDeleteBuffers(1, &bufferB); glDeleteBuffers(1, &bufferC);

    return isCorrect;
}

static bool testMatrixSquare(float eps = 1e-5f){
    MatOps matOps = MatOps();
    GLuint bufferA = 0;
    GLuint bufferB = 0;
    
    const int N = 4;
    float a[] = { -0.476f, 0.598f, 4.242f, -0.343f, 0.078f, 0.874f, -3.153f, 0.119f, 1.299f, 2.93f, -4.059f, -1.966f, -4.093f, 3.096f, 1.934f, -4.581f };
    float b[] = { 7.187477f, 11.605136f, -21.786326f, -6.534059f, -4.55177f, -8.059346f, 10.603327f, 5.730911f, 2.384413f, -14.641984f, 8.945305f, 16.889353f, 23.452055f, -8.257866f, -43.833954f, 18.95564f };

    glGenBuffers(1, &bufferA); glGenBuffers(1, &bufferB);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferA);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(a), &a[0], GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferB);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(b), nullptr, GL_STATIC_DRAW);

    matOps.multiply(bufferA, bufferA, bufferB, N, true);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferB);
    float result[N * N];
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(b), &result[0]);

    bool isCorrect = true;
    for (int i = 0; i < N * N; i++)
        isCorrect = isCorrect && (abs(b[i] - result[i]) < eps);

    glDeleteBuffers(1, &bufferA); glDeleteBuffers(1, &bufferB);

    return isCorrect;
}

TEST_CASE( "Matrix multiplicaton is computed", "[matrix_ops]" ) {
    REQUIRE( testMatrixMultiplication() );
    REQUIRE( testMatrixSquare() );
}


static bool testMatrixVector(float eps = 1e-5f){
    MatOps matOps = MatOps();
    GLuint bufferA = 0;
    GLuint bufferV = 0;
    GLuint bufferResult = 0;
    
    const int N = 4;
    float a[] = { -3.351f, 1.898f, 1.35f, -0.209f, -2.84f, 2.926f, 3.079f, 0.125f, 0.051f, -2.639f, -4.968f, -1.29f, 0.854f, -4.307f, 2.938f, -2.677f };
    float v[] = { -2.673f, -4.577f, 4.976f, 2.387f };
    float r[] = { 6.488794f, 9.818497f, -15.857618f, 25.659886f };

    glGenBuffers(1, &bufferA); glGenBuffers(1, &bufferV); glGenBuffers(1, &bufferResult);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferA);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(a), &a[0], GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferV);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(v), &v[0], GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferResult);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(r), nullptr, GL_STATIC_DRAW);

    matOps.matVec(bufferA, bufferV, bufferResult, N);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferResult);
    float result[N];
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(r), &result[0]);

    bool isCorrect = true;
    for (int i = 0; i < N; i++)
        isCorrect = isCorrect && (abs(r[i] - result[i]) < eps);

    glDeleteBuffers(1, &bufferA); glDeleteBuffers(1, &bufferV); glDeleteBuffers(1, &bufferResult);

    return isCorrect;
}

TEST_CASE( "Matrix and vector multiplicaton is computed", "[matrix_ops]" ) {
    REQUIRE( testMatrixVector() );
}

static bool testDot(float eps = 1e-4f){
    MatOps matOps = MatOps();
    GLuint bufferU = 0;
    GLuint bufferV = 0;
    GLuint bufferResult = 0;

    const int N = 1e6;
    vector<float> u = vector<float>(N);
    vector<float> v = vector<float>(N);
    float expectedDot = 0;
    for (int i = 0; i < N; i++)
    {
        float x = (float)rand() / RAND_MAX;
        float y = (float)rand() / RAND_MAX;
        u[i] = x - 0.5f;
        v[i] = y - 0.5f;
        expectedDot += u[i] * v[i];
    }

    glGenBuffers(1, &bufferU); glGenBuffers(1, &bufferV); glGenBuffers(1, &bufferResult);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferU);
    glBufferData(GL_SHADER_STORAGE_BUFFER, u.size() * sizeof(float), u.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferV);
    glBufferData(GL_SHADER_STORAGE_BUFFER, v.size() * sizeof(float), v.data(), GL_STATIC_DRAW);

    float initResult[4] = { -1.f, -1.f, -1.f, -1.f };
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferResult);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(initResult), initResult, GL_STATIC_DRAW);

    matOps.dot(bufferU, bufferV, bufferResult, N, 2);

    float result[4];
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(result), &result[0]);

    bool isCorrect = abs(expectedDot - result[2]) < abs(expectedDot * eps)
                   && result[0] == -1.f && result[1] == -1.f && result[3] == -1.f;

    Logger::log("Expected result: " + to_string(expectedDot) + ", result: " + to_string(result[2]), __LOG_DATA__);

    glDeleteBuffers(1, &bufferU); glDeleteBuffers(1, &bufferV); glDeleteBuffers(1, &bufferResult);

    return isCorrect;
}

static bool testDotMultiIndices(float eps = 1e-4f){
    MatOps matOps = MatOps();
    GLuint bufferU = 0;
    GLuint bufferV = 0;
    GLuint bufferResult = 0;

    const int N = 8;
    float u[] = { 1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f };
    float v[] = { 8.f, 7.f, 6.f, 5.f, 4.f, 3.f, 2.f, 1.f };
    float expectedDot = 120.f;

    glGenBuffers(1, &bufferU); glGenBuffers(1, &bufferV); glGenBuffers(1, &bufferResult);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferU);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(u), &u[0], GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferV);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(v), &v[0], GL_STATIC_DRAW);

    float initResult[4] = { -1.f, -1.f, -1.f, -1.f };
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferResult);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(initResult), initResult, GL_DYNAMIC_DRAW);

    matOps.dot(bufferU, bufferV, bufferResult, N, glm::ivec3(0, 2, -1));

    float result[4];
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(result), &result[0]);

    bool isCorrect = abs(expectedDot - result[0]) < eps
                   && abs(expectedDot - result[2]) < eps
                   && result[1] == -1.f && result[3] == -1.f;

    Logger::log("Expected result: " + to_string(expectedDot) + ", result: " + to_string(result[2]), __LOG_DATA__);

    glDeleteBuffers(1, &bufferU); glDeleteBuffers(1, &bufferV); glDeleteBuffers(1, &bufferResult);

    return isCorrect;
}

static bool testDotIndirect(float eps = 1e-4f){
    MatOps matOps = MatOps();
    GLuint bufferU = 0;
    GLuint bufferV = 0;
    GLuint bufferResult = 0;
    GLuint indirectBuffer = 0;

    const int N = (int)5e6;
    vector<float> u = vector<float>(N);
    vector<float> v = vector<float>(N);
    float expectedDot = 0;
    for (int i = 0; i < N; i++)
    {
        float x = (float)rand() / RAND_MAX;
        float y = (float)rand() / RAND_MAX;
        u[i] = x - 0.5f;
        v[i] = y - 0.5f;
        expectedDot += u[i] * v[i];
    }
    vector<DispatchParams> dispatchParams = {
        {GLuint((N + 127) / 128), 1, 1},
        {1, 1, 1}
    };

    glGenBuffers(1, &bufferU); glGenBuffers(1, &bufferV); glGenBuffers(1, &bufferResult); glGenBuffers(1, &indirectBuffer);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferU);
    glBufferData(GL_SHADER_STORAGE_BUFFER, u.size() * sizeof(float), u.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferV);
    glBufferData(GL_SHADER_STORAGE_BUFFER, v.size() * sizeof(float), v.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, indirectBuffer);
    glBufferData(GL_DISPATCH_INDIRECT_BUFFER, dispatchParams.size() * sizeof(DispatchParams), dispatchParams.data(), GL_STATIC_DRAW);

    float initResult[4] = { -1.f, -1.f, -1.f, -1.f };
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferResult);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(initResult), initResult, GL_DYNAMIC_DRAW);

    matOps.dotIndirect(bufferU, bufferV, bufferResult, indirectBuffer, 0, sizeof(DispatchParams), N, 2);
    ShaderProgram::indirectBarrier();

    float result[4];
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(result), &result[0]);

    bool isCorrect = abs(expectedDot - result[2]) < abs(expectedDot * eps)
                   && result[0] == -1.f && result[1] == -1.f && result[3] == -1.f;

    Logger::log("Expected result: " + to_string(expectedDot) + ", result: " + to_string(result[2]), __LOG_DATA__);

    glDeleteBuffers(1, &bufferU); glDeleteBuffers(1, &bufferV); glDeleteBuffers(1, &bufferResult); glDeleteBuffers(1, &indirectBuffer);

    return isCorrect;
}

TEST_CASE( "Dot product is computed", "[matrix_ops]" ) {
    REQUIRE( testDot(1e-3) );
    REQUIRE( testDotMultiIndices(1e-3) );
    REQUIRE( testDotIndirect(1e-3) );
}

static bool testSaxpyScalarOperations(float eps = 1e-3f){
    MatOps matOps = MatOps();
    GLuint bufferX = 0;
    GLuint bufferY = 0;
    GLuint bufferZ = 0;
    GLuint bufferAlpha = 0;

    const int N = 8;
    float x[] = { 4.478f, -1.791f, -3.52f, 1.262f, -0.746f, 2.41f, 1.622f, 0.472f };
    float y[] = { 3.159f, -3.042f, 2.829f, -3.174f, -0.205f, 0.557f, 0.436f, -3.879f };
    float z[] = { 10.796f, -7.875f, 2.138f, -5.086f, -1.156f, 3.524f, 2.494f, -7.286f };   // x + 2*y

    float scalars[16] = { 2.f };
    vector<int> alphaOperations(16, 0);
    alphaOperations[0] = 1;

    glGenBuffers(1, &bufferX); glGenBuffers(1, &bufferY); glGenBuffers(1, &bufferZ); glGenBuffers(1, &bufferAlpha);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferX);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(x), &x[0], GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferY);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(y), &y[0], GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferZ);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(z), nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferAlpha);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(scalars), &scalars[0], GL_STATIC_DRAW);

    matOps.saxpy(bufferX, bufferY, bufferZ, bufferAlpha, N, alphaOperations, true);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferZ);
    float result[N];
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(z), &result[0]);

    bool isCorrect = true;
    for (int i = 0; i < N; i++)
        isCorrect = isCorrect && (abs(z[i] - result[i]) < eps);

    glDeleteBuffers(1, &bufferX); glDeleteBuffers(1, &bufferY); glDeleteBuffers(1, &bufferZ); glDeleteBuffers(1, &bufferAlpha);

    return isCorrect;
}


static bool testSaxpyFloatAlpha(float eps = 1e-3f){
    MatOps matOps = MatOps();
    GLuint bufferX = 0;
    GLuint bufferY = 0;
    GLuint bufferZ = 0;

    const int N = 8;
    float x[] = { 4.478f, -1.791f, -3.52f, 1.262f, -0.746f, 2.41f, 1.622f, 0.472f };
    float y[] = { 3.159f, -3.042f, 2.829f, -3.174f, -0.205f, 0.557f, 0.436f, -3.879f };
    float alpha = -1.5f;
    float z[] = { -0.2605f, 2.772f, -7.7635f, 6.023f, -0.4385f, 1.5745f, 0.968f, 6.2905f };

    glGenBuffers(1, &bufferX); glGenBuffers(1, &bufferY); glGenBuffers(1, &bufferZ);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferX);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(x), &x[0], GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferY);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(y), &y[0], GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferZ);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(z), nullptr, GL_STATIC_DRAW);

    matOps.saxpy(bufferX, bufferY, bufferZ, alpha, N, true);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferZ);
    float result[N];
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(z), &result[0]);

    bool isCorrect = true;
    for (int i = 0; i < N; i++)
        isCorrect = isCorrect && (abs(z[i] - result[i]) < eps);

    glDeleteBuffers(1, &bufferX); glDeleteBuffers(1, &bufferY); glDeleteBuffers(1, &bufferZ);

    return isCorrect;
}

TEST_CASE( "Saxpy is computed", "[matrix_ops]" ) {
    REQUIRE( testSaxpyScalarOperations() );
    REQUIRE( testSaxpyFloatAlpha() );
}

static bool testCopy(float eps = 1e-5f){
    MatOps matOps = MatOps();
    GLuint bufferX = 0;
    GLuint bufferY = 0;

    const int N = 8;
    float x[] = { 4.478f, -1.791f, -3.52f, 1.262f, -0.746f, 2.41f, 1.622f, 0.472f };

    glGenBuffers(1, &bufferX); glGenBuffers(1, &bufferY);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferX);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(x), &x[0], GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferY);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(x), nullptr, GL_STATIC_DRAW);

    matOps.copy(bufferX, bufferY, N, true);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferY);
    float result[N];
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(x), &result[0]);

    bool isCorrect = true;
    for (int i = 0; i < N; i++)
        isCorrect = isCorrect && (abs(x[i] - result[i]) < eps);

    glDeleteBuffers(1, &bufferX); glDeleteBuffers(1, &bufferY);

    return isCorrect;
}

TEST_CASE( "Copy is computed", "[matrix_ops]" ) {
    REQUIRE( testCopy() );
}

static bool testTranspose(float eps = 1e-5f){
    MatOps matOps = MatOps();
    GLuint bufferA = 0;
    GLuint bufferResult = 0;

    const int N = 4;
    float mat[]  = { -0.408f, -2.196f, -2.233f, -0.786f, -0.864f, -2.935f, 1.856f, 4.312f, 4.48f, -3.197f, -2.682f, -0.931f, 0.946f, 1.929f, 0.894f, -0.07f };
    float matT[] = { -0.408f, -0.864f, 4.48f, 0.946f, -2.196f, -2.935f, -3.197f, 1.929f, -2.233f, 1.856f, -2.682f, 0.894f, -0.786f, 4.312f, -0.931f, -0.07f };

    glGenBuffers(1, &bufferA); glGenBuffers(1, &bufferResult);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferA);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(mat), &mat[0], GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferResult);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(matT), nullptr, GL_STATIC_DRAW);

    matOps.transpose(bufferA, bufferResult, N, true);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferResult);
    float result[N * N];
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(matT), &result[0]);

    bool isCorrect = true;
    for (int i = 0; i < N * N; i++)
        isCorrect = isCorrect && (abs(matT[i] - result[i]) < eps);

    glDeleteBuffers(1, &bufferA); glDeleteBuffers(1, &bufferResult);

    return isCorrect;
}

TEST_CASE( "Transpose is computed", "[matrix_ops]" ) {
    REQUIRE( testTranspose() );
}