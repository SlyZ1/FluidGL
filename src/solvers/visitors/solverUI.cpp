#include "solverUI.hpp"

#include "ui/utilsUI.hpp"
#include "solvers/flipSolverCPU.hpp"
#include "solvers/flipSolverGPU.hpp"

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


void SolverUI::visit(IParticleSolver& solver) {
    IParticleSolverConfig& config = static_cast<IParticleSolverConfig&>(solver.getDraftConfig());

    UtilsUI::BeginTwoColumnLayout();

    DRAW_FIELD(UtilsUI::drawInput, "Particle Number", int, PartN, 0, (int)1e8)
    DRAW_FIELD(UtilsUI::drawDrag, "Particle Radius", float, PartRadius, 1, 200)
    DRAW_FIELD(UtilsUI::drawDrag, "Timestep", float, Dt, 0.01f, 0.1f)

    UtilsUI::EndTwoColumnLayout();
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

    DRAW_FIELD(UtilsUI::drawDrag, "Domain Size", vec3, DomainSize, 0, 10000)
    ImGui::Dummy(ImVec2(0, 10));
    DRAW_FIELD(UtilsUI::drawDrag, "Particle Radius", float, PartRadius, 1, 200)
    DRAW_FIELD(UtilsUI::drawDrag, "Particle Per Cell Size", float, PartPerH, 1, 10)
    DRAW_FIELD(UtilsUI::drawDrag, "Density Multiplier", float, DensityMultiplier, 1, 10)
    ImGui::Dummy(ImVec2(0, 10));
    DRAW_FIELD(UtilsUI::drawDrag, "Flip Ratio", float, FlipRatio, 0.0f, 1.0f)
    DRAW_FIELD(UtilsUI::drawDrag, "Timestep", float, Dt, 0.01f, 0.1f)
    ImGui::Dummy(ImVec2(0, 10));

    vec3 initPos = (config.getFluidInitializer().min + config.getFluidInitializer().max) / 2.0f;
    vec3 initSize = config.getFluidInitializer().max - config.getFluidInitializer().min;
    vec3 dim = config.getDomainSize();

    UtilsUI::Label("Particle init Size");
    UtilsUI::drawDrag("Particle init Size", initSize, -1000, 1000);
    initPos = glm::clamp(initPos, (initSize-dim) * 0.5f, -(initSize-dim) * 0.5f);
    UtilsUI::Label("Particle Init Position");
    UtilsUI::drawDrag("Particle Position", initPos, -1000, 1000);
    initPos = glm::clamp(initPos, (initSize-dim) * 0.5f, -(initSize-dim) * 0.5f);

    config.setFluidInitializer({ initPos - initSize*0.5f, initPos + initSize*0.5f });

    ImGui::Dummy(ImVec2(0, 10));
    
    UtilsUI::EndTwoColumnLayout();

    if (UtilsUI::BeginCustomHeader("Forces")) {
        UtilsUI::BeginTwoColumnLayout();

        DRAW_FIELD(UtilsUI::drawDrag, "Gravity", float, Gravity, 0, 1000)
        ImGui::BeginDisabled(true);
        DRAW_FIELD(UtilsUI::drawDrag, "Surface Tension", float, Sigma, 0, 1000) // Still incomplete
        ImGui::EndDisabled();

        UtilsUI::EndTwoColumnLayout();
        ImGui::TreePop();
    } UtilsUI::EndCustomHeader();

    if (UtilsUI::BeginCustomHeader("CGS")) {
        UtilsUI::BeginTwoColumnLayout();

        DRAW_FIELD(UtilsUI::drawInput, "Max Iterations", int, CgMaxIter, 5, 300)
#define CUSTOM_DRAW(label, name, min, max) UtilsUI::drawDrag(label, name, min, max, "%.5f", 5e-5f)
        DRAW_FIELD(CUSTOM_DRAW, "Tolerance", float, CgTol, 1e-5f, 1e-1f)
#undef CUSTOM_DRAW

        UtilsUI::EndTwoColumnLayout();
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