#include "solverUI.hpp" 

#include "ui/utilsUI.hpp"
#include "solvers/flip/flipSolverCPU.hpp"
#include "solvers/flip/liquidFlipSolverGPU.hpp"
#include "solvers/flip/smokeFlipSolverGPU.hpp"
#include "solvers/initializers/cubeInitializer.hpp"
#include "solvers/initializers/sphereInitializer.hpp"

using namespace glm;
using namespace std;

vector<const char*> SolverUI::solverNames(){
    vector<const char*> names = {};
    for (SolverType type = (SolverType)0; type < SolverType::MaxType; type=(SolverType)((int)type+1))
        names.push_back(solverName(type));
    return names;
}

const char* SolverUI::solverName(SolverType type){
    switch (type)
    {
#define CASE(type, val) case SolverType::type: return #type;
        SOLVER_TYPE_ITER(CASE)
#undef CASE
    }
    return "";
}

#define DRAW_FIELD(drawingFunction, label, type, uppercasedName, min, max) \
    UtilsUI::Label(label); \
    type uppercasedName = config.get##uppercasedName(); \
    drawingFunction(#uppercasedName, uppercasedName, min, max); \
    config.set##uppercasedName(uppercasedName);

#define CHECK_BOX_DRAWING_FUNCTION(label, name, min, max) UtilsUI::drawCheckbox(label, name)

void SolverUI::drawInitializer(shared_ptr<FluidInitializer> initializer, vec3 dim, float scale){
    m_maxInitializerSize = dim;
    m_currentInitializerScale = scale;
    initializer->accept(*this);
}

void SolverUI::visit(IParticleSolver& solver) {
    IParticleSolverConfig& config = static_cast<IParticleSolverConfig&>(solver.getDraftConfig());

    UtilsUI::BeginTwoColumnLayout();

    DRAW_FIELD(UtilsUI::drawInput, "Particle Number", int, PartN, 0, (int)1e8)
    DRAW_FIELD(UtilsUI::drawDrag, "Particle Radius", float, PartRadius, 1, 200)
    DRAW_FIELD(UtilsUI::drawDrag, "Timestep", float, Dt, 0.01f, 0.1f)

    UtilsUI::EndTwoColumnLayout();
}

void SolverUI::visit(EulerSolverGPU&) {

}

void SolverUI::visit(FlipSolverCPU& solver) {
    visit(static_cast<IParticleSolver&>(solver));
    FlipSolverCPUConfig& config = static_cast<FlipSolverCPUConfig&>(solver.getDraftConfig());

    UtilsUI::BeginTwoColumnLayout();

    DRAW_FIELD(UtilsUI::drawDrag, "Particle Per Cell Size", float, PartPerH, 1, 10)
    DRAW_FIELD(UtilsUI::drawDrag, "Overrelaxation", float, Overrelaxation, 1, 2)
    DRAW_FIELD(UtilsUI::drawDrag, "Domain Size", vec2, DomainSize, 0, 10000)

    UtilsUI::EndTwoColumnLayout();
}

void SolverUI::visit(FlipSolverGPU& solver) {
    FlipSolverGPUConfig& config = static_cast<FlipSolverGPUConfig&>(solver.getDraftConfig());
    
    UtilsUI::BeginTwoColumnLayout();

    // GENERAL
    DRAW_FIELD(UtilsUI::drawDrag, "Domain Size", vec3, DomainSize, 0, 10000)
    ImGui::Dummy(ImVec2(0, 10));
    DRAW_FIELD(UtilsUI::drawDrag, "Particle Radius", float, PartRadius, 0.01f, 200)
    DRAW_FIELD(UtilsUI::drawDrag, "Particle Per Cell Size", float, PartPerH, 1, 10)
    DRAW_FIELD(UtilsUI::drawDrag, "Density Multiplier", float, DensityMultiplier, 0.01f, 10)
    ImGui::Dummy(ImVec2(0, 10));
    DRAW_FIELD(UtilsUI::drawDrag, "Flip Ratio", float, FlipRatio, 0.0f, 1.0f)
    DRAW_FIELD(UtilsUI::drawDrag, "Timestep", float, Dt, 0.01f, 0.1f)
    DRAW_FIELD(UtilsUI::drawDrag, "Num Push Appart", int, NumPushAppart, 0, 100)
    ImGui::Dummy(ImVec2(0, 10));

    // INITIALIZER
    ImGui::BeginDisabled(solver.getIsFull());
    UtilsUI::EndTwoColumnLayout();
    UtilsUI::TextWithShadow("Particles Initializer");
    UtilsUI::BeginTwoColumnLayout();
    drawInitializer(config.getInitializer(), config.getDomainSize(), config.getPartRadius());
    ImGui::EndDisabled();

    UtilsUI::EndTwoColumnLayout();
    
    ImGui::Dummy(ImVec2(0, 10));

    if (UtilsUI::BeginCustomHeader("Boundaries")){
        UtilsUI::BeginTwoColumnLayout();

        DRAW_FIELD(CHECK_BOX_DRAWING_FUNCTION, "Open X-|X+", bvec2, OpenBoundsX, 0, 0)
        DRAW_FIELD(CHECK_BOX_DRAWING_FUNCTION, "Open Y-|Y+", bvec2, OpenBoundsY, 0, 0)
        DRAW_FIELD(CHECK_BOX_DRAWING_FUNCTION, "Open Z-|Z+", bvec2, OpenBoundsZ, 0, 0)

        UtilsUI::EndTwoColumnLayout();
        ImGui::TreePop();
    } UtilsUI::EndCustomHeader();

    ImGui::Dummy(ImVec2(0, 10));

    // CGS
    if (UtilsUI::BeginCustomHeader("CGS")) {
        UtilsUI::BeginTwoColumnLayout();

        DRAW_FIELD(UtilsUI::drawInput, "Max Iterations", int, CgMaxIter, 5, 300)
#define CUSTOM_DRAW(label, name, min, max) UtilsUI::drawDrag(label, name, min, max, "%.5f", 5e-5f)
        DRAW_FIELD(CUSTOM_DRAW, "Tolerance", float, CgTol, 1e-5f, 1e-1f)
#undef CUSTOM_DRAW
        DRAW_FIELD(CHECK_BOX_DRAWING_FUNCTION, "Accuracy First", bool, AccuracyFirst, 0, 0)

        UtilsUI::EndTwoColumnLayout();
        ImGui::TreePop();
    } UtilsUI::EndCustomHeader();
}

void SolverUI::visit(LiquidFlipSolverGPU& solver) {
    LiquidFlipSolverGPUConfig& config = static_cast<LiquidFlipSolverGPUConfig&>(solver.getDraftConfig());

    // FORCES
    if (UtilsUI::BeginCustomHeader("Liquid")) {
        UtilsUI::BeginTwoColumnLayout();

        DRAW_FIELD(UtilsUI::drawDrag, "Gravity", float, Gravity, 0, 1000)

        UtilsUI::EndTwoColumnLayout();
        ImGui::TreePop();
    } UtilsUI::EndCustomHeader();

    if (UtilsUI::BeginCustomHeader("FLIP")){
        visit((FlipSolverGPU&)solver);
        ImGui::TreePop();
    } UtilsUI::EndCustomHeader();
}

void SolverUI::visit(SmokeFlipSolverGPU& solver) {
    SmokeFlipSolverGPUConfig& config = static_cast<SmokeFlipSolverGPUConfig&>(solver.getDraftConfig());

    if (UtilsUI::BeginCustomHeader("Smoke")){
        UtilsUI::BeginTwoColumnLayout();

        DRAW_FIELD(UtilsUI::drawDrag, "Buoyancy Alpha", float, FlotAlpha, 0, 100)
        DRAW_FIELD(UtilsUI::drawDrag, "Buoyancy Beta", float, FlotBeta, 0, 100)
        DRAW_FIELD(UtilsUI::drawDrag, "Density Decrease Rate", float, DensityDecreaseRate, 0, 100)
        DRAW_FIELD(UtilsUI::drawDrag, "Temperature Decrease Rate", float, TemperatureDecreaseRate, 0, 100)
        ImGui::Dummy(ImVec2(0, 5));
        DRAW_FIELD(UtilsUI::drawDrag, "Vorticity Factor", float, VorticityFactor, 0, 1.5)
        DRAW_FIELD(UtilsUI::drawSlider, "Diffusion Factor", float, DiffusionFactor, 0, 10)
        DRAW_FIELD(UtilsUI::drawDrag, "Divergence Factor", float, DivergenceFactor, 0, 1)
        
        ImGui::Dummy(ImVec2(0, 5));

        UtilsUI::EndTwoColumnLayout();
        UtilsUI::TextWithShadow("Smoke Source");
        UtilsUI::BeginTwoColumnLayout();
        drawInitializer(config.getDensitySource(), config.getDomainSize(), config.getPartRadius());
        
        UtilsUI::EndTwoColumnLayout();
        ImGui::TreePop();
    } UtilsUI::EndCustomHeader();

    if (UtilsUI::BeginCustomHeader("FLIP")){
        visit((FlipSolverGPU&)solver);
        ImGui::TreePop();
    } UtilsUI::EndCustomHeader();
}

void SolverUI::visit(XpbdSolverCPU& solver) {
    XpbdSolverCPUConfig& config = static_cast<XpbdSolverCPUConfig&>(solver.getDraftConfig());

    DRAW_FIELD(UtilsUI::drawDrag, "Timestep", float, Dt, 0.01f, 0.1f)
    ImGui::Dummy(ImVec2(0, 10));

    if (UtilsUI::BeginCustomHeader("Forces")) {
        UtilsUI::BeginTwoColumnLayout();

        DRAW_FIELD(UtilsUI::drawDrag, "Gravity", float, Gravity, 0, 1000)

        UtilsUI::EndTwoColumnLayout();
        ImGui::TreePop();
    } UtilsUI::EndCustomHeader();
}



void SolverUI::visit(CubeInitializer& initializer) {
    vec3 initPos = (initializer.min(m_currentInitializerScale) + initializer.max(m_currentInitializerScale)) / 2.0f;
    vec3 initSize = initializer.max(m_currentInitializerScale) - initializer.min(m_currentInitializerScale);

    initSize = glm::clamp(initSize, vec3(0), m_maxInitializerSize);
    UtilsUI::Label("Size");
    UtilsUI::drawDrag("Size", initSize, -1000, 1000);
    initSize = glm::clamp(initSize, vec3(0), m_maxInitializerSize);

    initPos = glm::clamp(initPos, (initSize - m_maxInitializerSize) * 0.5f, -(initSize - m_maxInitializerSize) * 0.5f);
    UtilsUI::Label("Position");
    UtilsUI::drawDrag("Position", initPos, -1000, 1000);
    initPos = glm::clamp(initPos, (initSize - m_maxInitializerSize) * 0.5f, -(initSize - m_maxInitializerSize) * 0.5f);

    initSize = glm::clamp(initSize, vec3(0), m_maxInitializerSize - 2.f * abs(initPos));

    initializer.setSizeAndPos(initSize, initPos, m_currentInitializerScale);
}

void SolverUI::visit(SphereInitializer& initializer) {
    vec3 initPos = (initializer.min(m_currentInitializerScale) + initializer.max(m_currentInitializerScale)) / 2.0f;
    float initRadius = (initializer.max(m_currentInitializerScale).x - initializer.min(m_currentInitializerScale).x) * 0.5f;
    float maxInitRadius = glm::min(m_maxInitializerSize.x, glm::min(m_maxInitializerSize.y, m_maxInitializerSize.z));
    
    initRadius = glm::clamp(initRadius, 0.0f, maxInitRadius);
    UtilsUI::Label("Radius");
    UtilsUI::drawDrag("Radius", initRadius, 0, 1000);
    initRadius = glm::clamp(initRadius, 0.0f, maxInitRadius);
    vec3 initSize = vec3(initRadius * 2.0f);
    
    initPos = glm::clamp(initPos, (initSize - m_maxInitializerSize) * 0.5f, -(initSize - m_maxInitializerSize) * 0.5f);
    UtilsUI::Label("Position");
    UtilsUI::drawDrag("Position", initPos, -1000, 1000);
    initPos = glm::clamp(initPos, (initSize - m_maxInitializerSize) * 0.5f, -(initSize - m_maxInitializerSize) * 0.5f);

    initSize = glm::clamp(initSize, vec3(0), m_maxInitializerSize - 2.f * abs(initPos));
    initRadius = glm::min(initSize.x, glm::min(initSize.y, initSize.z)) * 0.5f;

    initializer.setRadiusAndPos(initRadius, initPos, m_currentInitializerScale);
}

#undef DRAW_FIELD
#undef CHECK_BOX_DRAWING_FUNCTION