#include "solverUI.hpp"

#include "ui/utilsUI.hpp"
#include "solvers/flip/flipSolverCPU.hpp"
#include "solvers/flip/liquidFlipSolverGPU.hpp"
#include "solvers/flip/smokeFlipSolverGPU.hpp"

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

FluidInitializer SolverUI::drawFluidInitializer(FluidInitializer initializer, vec3 dim, float scale){
    vec3 initPos = (initializer.min(scale) + initializer.max(scale)) / 2.0f;
    vec3 initSize = initializer.max(scale) - initializer.min(scale);

    initSize = glm::clamp(initSize, vec3(0), dim);
    UtilsUI::Label("Particle init Size");
    UtilsUI::drawDrag("Particle init Size", initSize, -1000, 1000);
    initSize = glm::clamp(initSize, vec3(0), dim);

    initPos = glm::clamp(initPos, (initSize-dim) * 0.5f, -(initSize-dim) * 0.5f);
    UtilsUI::Label("Particle Init Position");
    UtilsUI::drawDrag("Particle Position", initPos, -1000, 1000);
    initPos = glm::clamp(initPos, (initSize-dim) * 0.5f, -(initSize-dim) * 0.5f);

    initSize = glm::clamp(initSize, vec3(0), dim - 2.f * abs(initPos));

    return { initPos - initSize*0.5f, initPos + initSize*0.5f, scale };
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
    FluidInitializer newInitializer = drawFluidInitializer(config.getFluidInitializer(), config.getDomainSize(), config.getPartRadius());
    config.setFluidInitializer(newInitializer);
    ImGui::EndDisabled();

    ImGui::Dummy(ImVec2(0, 10));
    
    UtilsUI::EndTwoColumnLayout();

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
        ImGui::Dummy(ImVec2(0, 5));
        DRAW_FIELD(UtilsUI::drawDrag, "Density Decrease Rate", float, DensityDecreaseRate, 0, 100)
        DRAW_FIELD(UtilsUI::drawDrag, "Temperature Decrease Rate", float, TemperatureDecreaseRate, 0, 100)
        
        ImGui::Dummy(ImVec2(0, 5));

        FluidInitializer newInitializer = drawFluidInitializer(config.getDensitySource(), config.getDomainSize(), config.getPartRadius());
        config.setDensitySource(newInitializer);
        
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

#undef DRAW_FIELD
#undef CHECK_BOX_DRAWING_FUNCTION