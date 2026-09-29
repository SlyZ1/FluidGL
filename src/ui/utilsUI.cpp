#include "utilsUI.hpp"

#include "colorsUI.hpp"

using namespace std;

bool UtilsUI::s_isHeaderLight = true;

void UtilsUI::TextWithShadow(const char* text, ImVec4 textColor, ImVec4 shadowColor, ImVec2 offset) {
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float rowHeight = ImGui::GetTextLineHeightWithSpacing();
    float textHeight = ImGui::GetTextLineHeight();

    pos.y += (rowHeight - textHeight) * 0.5f;
    ImDrawList* draw = ImGui::GetWindowDrawList();
    
    draw->AddText(ImVec2(pos.x + offset.x, pos.y + offset.y), ImGui::GetColorU32(shadowColor), text);
    draw->AddText(pos, ImGui::GetColorU32(textColor), text);
    
    ImVec2 textSize = ImGui::CalcTextSize(text);
    ImGui::Dummy(textSize);
}

void UtilsUI::renderToolTip(const string& tip) {
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(300.0f);
        ImGui::TextUnformatted(tip.c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void UtilsUI::AlignInputToRight(const char* input) {
    auto posX = (ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(input).x 
    - ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x);
    if(posX > ImGui::GetCursorPosX())
        ImGui::SetCursorPosX(posX);
}

void UtilsUI::BeginTwoColumnLayout(float columnRatio)
{
    float availWidth = ImGui::GetContentRegionAvail().x;
    float labelWidth = std::max(availWidth * columnRatio, 120.0f);
    ImGui::BeginTable("##layout", 2, ImGuiTableFlags_SizingStretchProp);
    ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, labelWidth);
    ImGui::TableSetupColumn("Input", ImGuiTableColumnFlags_WidthStretch);
}

void UtilsUI::EndTwoColumnLayout()
{
    ImGui::EndTable();
}

bool UtilsUI::BeginCustomHeader(const string& name) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, s_isHeaderLight ? ColorsUI::fgColor : ColorsUI::mgColor);
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0.0f);
    ImGui::BeginChild((name + "_group").c_str(), ImVec2(-FLT_MIN, 0), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysUseWindowPadding);
    bool open = ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth);
    if (open){
        ImGui::Spacing();
        ImGui::Spacing();
    }
    s_isHeaderLight = !s_isHeaderLight;
    return open;
}

void UtilsUI::EndCustomHeader() {
    ImGui::EndChild();
    ImGui::PopStyleVar(1);
    ImGui::PopStyleColor(1);
    s_isHeaderLight = !s_isHeaderLight;
}

void UtilsUI::Label(const char* label, const string& desc, function<void(void)> customWidget, float widgetSize)
{
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    TextWithShadow(label, ImVec4(1,1,1,1), ImVec4(0.05f,0.05f,0.05f,0.7f), ImVec2(1,1));
    if (!desc.empty())
        renderToolTip(desc); 
    if (customWidget != nullptr){
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - widgetSize);
        customWidget();
    }
    ImGui::TableSetColumnIndex(1);
    ImGui::SetNextItemWidth(-FLT_MIN); 
}
