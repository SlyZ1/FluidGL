#include "rendererUI.hpp"

#include "renderers/liquidRenderer3D.hpp"
#include "renderers/smokeRenderer3D.hpp"
#include "renderers/triangleRenderer3D.hpp"
#include "renderers/smokeRayMarcher.hpp"

#include "ui/utilsUI.hpp"

using namespace glm;
using namespace std;

#define DRAW_FIELD(drawingFunction, label, type, uppercasedName, min, max) \
    UtilsUI::Label(label); \
    type uppercasedName = renderer.get##uppercasedName(); \
    drawingFunction(#uppercasedName, uppercasedName, min, max); \
    renderer.set##uppercasedName(uppercasedName);

void RendererUI::visit(TriangleRenderer& renderer){

}

void RendererUI::visit(SmokeRenderer3D& renderer){

}

void RendererUI::visit(LiquidRenderer3D& renderer){

}

void RendererUI::visit(SmokeRayMarcher& renderer){
    UtilsUI::BeginTwoColumnLayout();
    DRAW_FIELD(UtilsUI::drawSlider, "Absorption", float, Absorption, 0, 1)
    DRAW_FIELD(UtilsUI::drawSlider, "Light Absorption", float, LightAbsorption, 0, 1)
    ImGui::Dummy(ImVec2(0, 5));
    DRAW_FIELD(UtilsUI::drawDrag, "Smoke Step Ratio", float, NumStepSizeRatio, 0.1f, 20.0f)
    DRAW_FIELD(UtilsUI::drawDrag, "Light Step Ratio", float, NumLightStepSizeRatio, 0.1f, 20.0f)
    ImGui::Dummy(ImVec2(0, 5));
    DRAW_FIELD(UtilsUI::drawDrag, "Shadow Multiplier", float, ShadowMultiplier, 0.0f, 1.0f)
    ImGui::Dummy(ImVec2(0, 5));
    DRAW_FIELD(UtilsUI::drawDrag, "Light Direction", vec3, LightDir, -1.0f, 1.0f)

    UtilsUI::EndTwoColumnLayout();
}
