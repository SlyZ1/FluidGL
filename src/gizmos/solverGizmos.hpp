#ifndef SOLVER_GIZMOS_HPP
#define SOLVER_GIZMOS_HPP

#include <glm/glm.hpp>

#include "solvers/solverVisitor.hpp"
#include "solvers/solverManager.hpp"
#include "gizmos/wireframes.hpp"
#include "core/camera.hpp"


class SolverGizmos : public ISolverVisitor {
private:
    template<typename T>
    struct WireframeData {
        WireframeIndex index;
        T data;
        WireframeData() {
            index = -1;
            data = {};
        }
        WireframeData(WireframeIndex index_, T data_) : index(index_), data(data_) {}
    };

    std::unique_ptr<Wireframes> m_wireframes;
    const glm::vec4 m_domainColor = glm::vec4(0.29f, 0.769f, 0.282f, 1.0f);
    const glm::vec4 m_initializerColor = glm::vec4(0.859f, 0.529f, 0.318f, 1.0f);

    WireframeData<glm::vec3> m_solverDomainData = {};
    WireframeData<FluidInitializer> m_solverInitializerData = {};

    SolverType m_solverType = SolverType::MaxType;

    void resetWireframes();

public:
    SolverGizmos(std::weak_ptr<Camera> camera);
    ~SolverGizmos() override;

    SolverGizmos(const SolverGizmos&) = delete;
    SolverGizmos& operator=(const SolverGizmos&) = delete;
    SolverGizmos(SolverGizmos&&) = default;
    SolverGizmos& operator=(SolverGizmos&&) = default;

    void visit(IParticleSolver& solver) override;
    void visit(FlipSolverCPU& solver) override;
    void visit(FlipSolverGPU& solver) override;

    void render() const;
};

#endif