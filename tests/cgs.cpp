#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <vector>
#include "src/helpers/logger.hpp"
#include "src/cgs/cgs.hpp"

using namespace std;

namespace {

constexpr int N = 5;

const vector<float> kA = {
    36.987f, -10.254f,   2.168f,  -5.723f,   5.91f,
   -10.254f,  35.349f,  -4.102f,   4.83f,   -1.345f,
     2.168f,  -4.102f,  31.451f,  -2.339f, -10.968f,
    -5.723f,   4.83f,   -2.339f,  33.895f,  -1.718f,
     5.91f,   -1.345f, -10.968f,  -1.718f,  53.52f };   // symétrique définie positive

const vector<float> kXRef = { -2.683f, 0.02f, 2.47f, -3.049f, 2.646f };

vector<float> matVec(const vector<float>& v) {
    vector<float> out(N, 0.f);
    for (int i = 0; i < N; i++) {
        double s = 0;
        for (int j = 0; j < N; j++) s += (double)kA[i * N + j] * v[j];
        out[i] = (float)s;
    }
    return out;
}

float errorNorm(const vector<float>& x, const vector<float>& ref) {
    double s = 0;
    for (int i = 0; i < N; i++) {
        if (!std::isfinite(x[i])) return INFINITY;      // NaN / Inf = échec
        s += (double)(x[i] - ref[i]) * (x[i] - ref[i]);
    }
    return (float)std::sqrt(s);
}

void upload(GLuint buf, const vector<float>& v) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buf);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, v.size() * sizeof(float), v.data());
}

vector<float> download(GLuint buf) {
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
    vector<float> out(N);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buf);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, N * sizeof(float), out.data());
    return out;
}

// init() est appelé UNE seule fois, comme dans FlipSolverGPU
struct System {
    GLuint A = 0, b = 0, x = 0;
    CGS cgs;

    System(const vector<float>& b0, const vector<float>& x0) {
        glGenBuffers(1, &A); glGenBuffers(1, &b); glGenBuffers(1, &x);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, A);
        glBufferData(GL_SHADER_STORAGE_BUFFER, kA.size() * sizeof(float), kA.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, b);
        glBufferData(GL_SHADER_STORAGE_BUFFER, N * sizeof(float), b0.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, x);
        glBufferData(GL_SHADER_STORAGE_BUFFER, N * sizeof(float), x0.data(), GL_DYNAMIC_DRAW);
        cgs.init(N, A, b, x);
    }
    ~System() { glDeleteBuffers(1, &A); glDeleteBuffers(1, &b); glDeleteBuffers(1, &x); }
    System(const System&) = delete;
    System& operator=(const System&) = delete;

    void solve(int iterations, float tol = 1e-7f) { cgs.solve(iterations, tol); }
    vector<float> result() { return download(x); }
};

} // namespace

// 1) Une solution convergée doit rester convergée au solve suivant (pas de remise à zéro)
TEST_CASE("CGS keeps the previous solution between solves", "[cgs][warmstart]") {
    vector<float> b = matVec(kXRef);
    System sys(b, vector<float>(N, 0.f));

    sys.solve(100);
    REQUIRE(errorNorm(sys.result(), kXRef) < 1e-3f);    // sanity : bien convergé

    sys.solve(1);                                       // 1 seule itération, SANS réécrire x
    vector<float> r = sys.result();
    INFO("après le 2e solve : x[0] = " << r[0]);
    REQUIRE(errorNorm(r, kXRef) < 1e-3f);               // échoue si x est remis à 0, ou si NaN
}

// 2) Le cas réel : b change un peu à chaque frame, x de la frame précédente sert de départ
TEST_CASE("CGS warm start beats cold start after a small change of b", "[cgs][warmstart]") {
    vector<float> xNew = kXRef;
    for (int i = 0; i < N; i++) xNew[i] += 0.05f * ((i % 2) ? -1.f : 1.f);   // petite perturbation
    vector<float> bNew = matVec(xNew);

    // Warm : même objet, mêmes buffers, juste b réécrit (pas de nouvel init)
    System warm(matVec(kXRef), vector<float>(N, 0.f));
    warm.solve(100);
    upload(warm.b, bNew);
    warm.solve(2);
    float errWarm = errorNorm(warm.result(), xNew);

    // Cold : tout neuf, x = 0, même nombre d'itérations
    System cold(bNew, vector<float>(N, 0.f));
    cold.solve(2);
    float errCold = errorNorm(cold.result(), xNew);

    INFO("erreur warm = " << errWarm << ", erreur cold = " << errCold);
    REQUIRE(errCold > 1e-2f);                           // sanity : 2 itérations à froid ne suffisent pas
    REQUIRE(errWarm < 0.25f * errCold);
}

// 3) solve() doit lire son point de départ dans le buffer x (pas dans une copie interne)
TEST_CASE("CGS reads its initial guess from the x buffer", "[cgs][warmstart]") {
    vector<float> b = matVec(kXRef);
    System sys(b, vector<float>(N, 0.f));
    sys.solve(100);

    upload(sys.x, vector<float>(N, 0.f));               // on efface x depuis l'extérieur
    sys.solve(1);
    REQUIRE(errorNorm(sys.result(), kXRef) > 0.1f);     // ne doit PAS être resté convergé

    upload(sys.x, kXRef);                               // on réinjecte la solution
    sys.solve(1);
    REQUIRE(errorNorm(sys.result(), kXRef) < 1e-3f);
}