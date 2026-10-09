#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <algorithm>
#include <numeric>
#include <random>
#include <vector>
#include <cstdint>
#include "src/helpers/logger.hpp"
#include "src/core/shader_program.hpp"
#include "src/helpers/utils.hpp"

using namespace std;



/////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                     //
//                                        CLAUDE GENERATED                                             //
//                                                                                                     //
//                                                                                                     //
/////////////////////////////////////////////////////////////////////////////////////////////////////////


// Doit être identique à la valeur utilisée dans FlipSolverGPU
static constexpr int SHARED_SIZE = 1024;   // <-- à adapter
static const string SHADERS = "src/shaders/FLIP";

// ---------- Reconstruction de prefixSum ----------
struct PrefixSumGPU {
    ShaderProgram localSum, smallSum, globalSum;
    vector<GLuint> blockSums;      // un buffer par niveau de récursion

    PrefixSumGPU() {
        load(localSum,  "/prefixsum/localsum.glsl");
        load(smallSum,  "/prefixsum/smallsum.glsl");
        load(globalSum, "/prefixsum/globalsum.glsl");
    }
    ~PrefixSumGPU() {
        for (GLuint b : blockSums) glDeleteBuffers(1, &b);
    }

    static void load(ShaderProgram& p, const string& path) {
        p.create();
        p.load(GL_COMPUTE_SHADER, Utils::joinPath(SHADERS, path));
        p.link();
    }

    // Crée à la demande le buffer de sommes du niveau `level`
    GLuint blockSumBuffer(int level, int count) {
        while ((int)blockSums.size() <= level) {
            GLuint b = 0;
            glGenBuffers(1, &b);
            blockSums.push_back(b);
        }
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, blockSums[level]);
        glBufferData(GL_SHADER_STORAGE_BUFFER, count * sizeof(uint32_t), nullptr, GL_DYNAMIC_DRAW);
        return blockSums[level];
    }

    void run(GLuint data, int n, int level = 0) {
        int newN = (n + 511) / 512;
        GLuint blockSum = blockSumBuffer(level, newN);

        // 1) scan local dans chaque workgroup, total du bloc dans blockSum
        localSum.use();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, blockSum);
        localSum.dispatch(newN);
        ShaderProgram::SSBOBarrier();

        // 2) scan des sommes de blocs
        if (newN < SHARED_SIZE) {
            smallSum.use();
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, blockSum);
            glUniform1i(ShaderProgram::getVarLoc("length"), newN);
            smallSum.dispatch(1);
        } else {
            run(blockSum, newN, level + 1);
        }
        ShaderProgram::SSBOBarrier();

        // 3) ajout des sommes de blocs scannées
        globalSum.use();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, blockSum);
        glUniform1i(ShaderProgram::getVarLoc("length"), n);
        globalSum.dispatch(newN);
        ShaderProgram::SSBOBarrier();
    }
};

// ---------- Helpers ----------
static vector<uint32_t> randomData(size_t n, uint32_t maxValue, uint32_t seed) {
    mt19937 rng(seed);
    uniform_int_distribution<uint32_t> dist(0, maxValue);
    vector<uint32_t> v(n);
    for (auto& x : v) x = dist(rng);
    return v;
}

// Exécute le scan GPU sur `input` (rempli de zéros jusqu'à un multiple de 512, comme le solver)
static vector<uint32_t> gpuScan(PrefixSumGPU& ps, const vector<uint32_t>& input) {
    int n       = (int)input.size();
    int padded  = ((n + 511) / 512) * 512;
    vector<uint32_t> data(padded, 0);
    copy(input.begin(), input.end(), data.begin());

    GLuint buf = 0;
    glGenBuffers(1, &buf);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buf);
    glBufferData(GL_SHADER_STORAGE_BUFFER, padded * sizeof(uint32_t), data.data(), GL_DYNAMIC_DRAW);

    ps.run(buf, padded);

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
    vector<uint32_t> out(padded);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buf);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, padded * sizeof(uint32_t), out.data());
    glDeleteBuffers(1, &buf);

    out.resize(n);
    return out;
}

// Retourne l'indice du premier écart, ou n si tout est bon
static size_t firstMismatch(const vector<uint32_t>& a, const vector<uint32_t>& b) {
    for (size_t i = 0; i < a.size(); i++)
        if (a[i] != b[i]) return i;
    return a.size();
}

// ---------- Tests ----------
TEST_CASE("prefixSum matches CPU exclusive scan on random arrays", "[prefixsum]") {
    PrefixSumGPU ps;

    const int n = GENERATE(
        1, 2, 511, 512, 513, 1000, 1024, 5000, 65536,
        512 * SHARED_SIZE - 1,        // juste avant la récursion
        512 * SHARED_SIZE,            // seuil de la récursion
        512 * SHARED_SIZE + 1,
        512 * SHARED_SIZE * 2 + 37    // 2 niveaux de récursion
    );
    const uint32_t seed = GENERATE(1u, 2u, 3u);

    // petites valeurs (type "comptes par cellule")
    auto input = randomData(n, 4, seed);

    vector<uint32_t> expected(n);
    exclusive_scan(input.begin(), input.end(), expected.begin(), 0u);

    auto result = gpuScan(ps, input);

    size_t bad = firstMismatch(result, expected);
    INFO("n = " << n << ", seed = " << seed);
    INFO("premier écart à l'indice " << bad
         << (bad < (size_t)n ? " (bloc " + to_string(bad / 512) + ")" : ""));
    REQUIRE(bad == (size_t)n);
}

TEST_CASE("prefixSum with large values wraps like uint32", "[prefixsum]") {
    PrefixSumGPU ps;
    const int n = 512 * 8 + 100;
    auto input = randomData(n, 1u << 20, 99);   // la somme dépasse 2^32 : débordement identique CPU/GPU

    vector<uint32_t> expected(n);
    exclusive_scan(input.begin(), input.end(), expected.begin(), 0u);

    REQUIRE(firstMismatch(gpuScan(ps, input), expected) == (size_t)n);
}

TEST_CASE("prefixSum edge patterns", "[prefixsum]") {
    PrefixSumGPU ps;
    const int n = 512 * 4 + 3;

    SECTION("zeros") {
        vector<uint32_t> in(n, 0);
        for (uint32_t v : gpuScan(ps, in)) REQUIRE(v == 0);
    }
    SECTION("ones -> result[i] == i") {
        vector<uint32_t> in(n, 1);
        auto r = gpuScan(ps, in);
        for (int i = 0; i < n; i++) REQUIRE(r[i] == (uint32_t)i);
    }
    SECTION("single spike") {
        vector<uint32_t> in(n, 0);
        in[700] = 5;
        auto r = gpuScan(ps, in);
        for (int i = 0; i < n; i++) REQUIRE(r[i] == (i > 700 ? 5u : 0u));
    }
}