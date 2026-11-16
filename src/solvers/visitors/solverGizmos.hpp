#ifndef SOLVER_GIZMOS_HPP
#define SOLVER_GIZMOS_HPP

#include <glm/glm.hpp>

#include "solvers/solverType.hpp"
#include "solvers/initializers/fluidInitializer.hpp"
#include "solvers/visitors/solverVisitor.hpp"
#include "gizmos/wireframes.hpp"
#include "core/camera.hpp"

class SolverGizmos : public ISolverVisitor, public IInitializerVisitor, public IStatsProvider {
private:
    struct InitializerBounds {
        glm::vec3 min;
        glm::vec3 max;
        bool operator==(const InitializerBounds&) const = default;

        InitializerBounds() = default;
        InitializerBounds(const FluidInitializer& initializer){
            min = initializer.min(1.0f);
            max = initializer.max(1.0f);
        }
    };

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
    const glm::vec4 m_domainColor = glm::vec4(1.0f);
    const glm::vec4 m_initializerColor = glm::vec4(0.859f, 0.529f, 0.318f, 1.0f);

    float m_currentPartRadius = 1.0f;
    WireframeData<InitializerBounds>* m_currentInitializerData = nullptr;

    WireframeData<glm::vec3> m_solverDomainData = {};
    WireframeData<InitializerBounds> m_solverInitializerData = {};
    WireframeData<InitializerBounds> m_smokeDensitySourceData = {};

    SolverType m_solverType = SolverType::MaxType;

    StatIndex m_renderStatIndex = 0;
    GPUTimer m_renderTimer = {};

    void resetWireframes();
    void drawInitializerWireframes(std::shared_ptr<FluidInitializer> initializer, WireframeData<InitializerBounds>& initializerData, float partRadius);

public:
    SolverGizmos(std::weak_ptr<Camera> camera);
    ~SolverGizmos() override;

    SolverGizmos(const SolverGizmos&) = delete;
    SolverGizmos& operator=(const SolverGizmos&) = delete;
    SolverGizmos(SolverGizmos&&) = default;
    SolverGizmos& operator=(SolverGizmos&&) = default;

    void visit(IParticleSolver& solver) override;
    void visit(EulerSolverGPU& solver) override;
    void visit(FlipSolverCPU& solver) override;
    void visit(FlipSolverGPU& solver) override;
    void visit(LiquidFlipSolverGPU& solver) override;
    void visit(SmokeFlipSolverGPU& solver) override;
    void visit(XpbdSolverCPU& solver) override;

    void visit(CubeInitializer& initializer) override;
    void visit(SphereInitializer& initializer) override;

    void render() const;
};

#endif