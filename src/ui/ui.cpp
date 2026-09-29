#include "ui.hpp" 
#include "colorsUI.hpp"
#include <format>

using namespace std;

UI::UI(UIContext ctx) : m_ctx(std::move(ctx)) {}

void UI::setStatsContext(const vector<weak_ptr<IStatsProvider>>& ctx){
    m_statsCtx = vector<weak_ptr<Stats>>(ctx.size());
    for (int i = 0; i < (int)ctx.size(); i++){
        if (auto stats = ctx[i].lock()) 
            m_statsCtx[i] = stats->getStats();
    }
}

// ----------------------------------------- STATS -----------------------------------------

void UI::drawTimers(StatsPrimitive<float> timers){
    Stat<TimerValue> totalTimeStat = timers.get(0);
    if (timers.numStats() <= 2){
        UtilsUI::BeginTwoColumnLayout();
        UtilsUI::Label(totalTimeStat.label.c_str());
        string text = Utils::formatFloat(totalTimeStat.value, 2) + "ms";
        UtilsUI::AlignInputToRight(text.c_str());
        ImGui::Text("%s", text.c_str());
        UtilsUI::EndTwoColumnLayout();
    }
    else {
        ImGui::Indent();

        string headerName = "Latencies: " + Utils::formatFloat(totalTimeStat.value, 2) + "ms###latencies";
        if (!ImGui::CollapsingHeader(headerName.c_str())) { ImGui::Unindent(); return; }

        UtilsUI::BeginTwoColumnLayout();
        vector<StatIndex> timerPermutation = timers.getSortPermutation();
        float totalStatTime = timers.get(timerPermutation[0]).value;
        for (const StatIndex& i : timerPermutation)
        {
            if (i == 0) continue;
            Stat<TimerValue> stat = timers.get(i);
            UtilsUI::Label(stat.label.c_str());
            string percentage = " (" + to_string(glm::clamp((int)(100 * stat.value / totalStatTime), 0, 100)) + "%)";
            if (i == timerPermutation[0]) percentage = "";
            string text = Utils::formatFloat(stat.value, 2) + "ms" + percentage;
            UtilsUI::AlignInputToRight(text.c_str());
            ImGui::Text("%s", text.c_str());
        }
        UtilsUI::EndTwoColumnLayout();
        ImGui::Unindent();
    }
}

void UI::drawCounters(StatsPrimitive<int> counters){
    if (counters.numStats() > 1){
        ImGui::Indent();
        if (!ImGui::CollapsingHeader("Counters")) { ImGui::Unindent(); return; }
    }

    UtilsUI::BeginTwoColumnLayout();
    for (StatIndex i = 0; i < counters.numStats(); i++)
    {
        Stat<CounterValue> stat = counters.get(i);
        UtilsUI::Label(stat.label.c_str());
        string text = stat.value >= 10000 ? Utils::formatFloat(stat.value, 2, true) : to_string(stat.value);
        UtilsUI::AlignInputToRight(text.c_str());
        ImGui::Text("%s", text.c_str());
    }
    UtilsUI::EndTwoColumnLayout();

    if (counters.numStats() > 1) ImGui::Unindent();
}

void UI::drawStorages(StatsPrimitive<int> storages){
    if (storages.numStats() > 1){
        ImGui::Indent();
        if (!ImGui::CollapsingHeader("Storages")) { ImGui::Unindent(); return; }
    }

    UtilsUI::BeginTwoColumnLayout();
    for (StatIndex i = 0; i < storages.numStats(); i++)
    {
        Stat<StorageValue> stat = storages.get(i);
        UtilsUI::Label(stat.label.c_str());
        string suffix = Metrics::storageSuffix(stat.value);
        string text = to_string(stat.value) + suffix;
        UtilsUI::AlignInputToRight(text.c_str());
        ImGui::Text("%s", text.c_str());
    }
    UtilsUI::EndTwoColumnLayout();

    if (storages.numStats() > 1) ImGui::Unindent();
}

void UI::renderStats(){
    ImGuiIO& io = ImGui::GetIO();
    ImGuiWindowFlags flags =  ImGuiWindowFlags_AlwaysAutoResize
                            | ImGuiWindowFlags_NoCollapse
                            | ImGuiWindowFlags_NoDecoration
                            | ImGuiWindowFlags_NoMove
                            | ImGuiWindowFlags_NoResize;
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x, ImGui::GetMainViewport()->WorkPos.y), ImGuiCond_Always, ImVec2(1, 0));
    ImGui::SetNextWindowSize(ImVec2(270.f, 0.f));
    if (ImGui::Begin("Stats", nullptr, flags)) {
        for (const weak_ptr<Stats>& statsPtr : m_statsCtx)
        {
            auto stats = statsPtr.lock();
            if (!stats) continue;

            if (ImGui::CollapsingHeader(stats->name.c_str())) {
                drawTimers(stats->getTimers());
                drawCounters(stats->getCounters());
                drawStorages(stats->getStorages());
            }
            if (stats->numStats() > 0) ImGui::Dummy(ImVec2(0, 2.f));
        }
    }
    ImGui::End();
}


// ----------------------------------------- PARAMS -----------------------------------------


void UI::renderParams(){
    float padding = ImGui::GetStyle().WindowPadding.y;

    ImGui::BeginChild("Parameters", ImVec2(-FLT_MIN, -FLT_MIN), ImGuiChildFlags_Borders);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 4.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, 6.0f);
    ImGui::PushStyleColor(ImGuiCol_Border, ColorsUI::lightBlueBorder);
    ImGui::BeginChild("Scrollable Parameters", ImVec2(-FLT_MIN, -padding), ImGuiChildFlags_None, ImGuiWindowFlags_AlwaysVerticalScrollbar);

    auto solverManagerLocked = m_ctx.solverManager.lock();
    if (UtilsUI::BeginCustomHeader("Solver") && solverManagerLocked){

        UtilsUI::BeginTwoColumnLayout();

        static int selectedItem = solverManagerLocked->getCurrentType();
        UtilsUI::Label("Solver Used");
        vector<const char*> names = SolverUI::solverNames();
        if (ImGui::Combo("##Solver Used", &selectedItem, names.data(), names.size())){
            solverManagerLocked->instantiate((SolverType)selectedItem);
        }

        UtilsUI::EndTwoColumnLayout();
        ImGui::Dummy(ImVec2(0, 10));

        if (auto solver = solverManagerLocked->getSolver().lock()){
            solver->accept(solverUI);
            ImGui::Dummy(ImVec2(0, 5));
            if (ImGui::Button("Apply Changes", ImVec2(-FLT_MIN, 20)))
                solver->applyDraftConfig();
        }
        ImGui::TreePop();
    } UtilsUI::EndCustomHeader();

    if (UtilsUI::BeginCustomHeader("Renderer")){
        UtilsUI::BeginTwoColumnLayout();

        UtilsUI::EndTwoColumnLayout();
        ImGui::TreePop();
    } UtilsUI::EndCustomHeader();

    ImGui::EndChild();
    ImGui::PopStyleColor(1);
    ImGui::PopStyleVar(2);
    ImGui::EndChild();
}


// ----------------------------------------- MAIN -----------------------------------------

void UI::render(){ 
    float viewPortVerticalPos = ImGui::GetMainViewport()->WorkPos.y;
    float viewPortVerticalSize = ImGui::GetMainViewport()->WorkSize.y;

    ImGui::PushStyleColor(ImGuiCol_Header, ColorsUI::mgColor);
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ColorsUI::mgColor);
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ColorsUI::mgColor);
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ColorsUI::mgColor);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 0.0f);

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ColorsUI::fgColor);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);

        renderStats();

        ImGui::PopStyleColor(1);
        ImGui::PopStyleVar(1);

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ColorsUI::bgColor);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ColorsUI::mgColor);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.1f, 0.1f, 0.1f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.1f, 0.1f, 0.1f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 1.0f));
        
            ImGui::SetNextWindowPos(ImVec2(0, viewPortVerticalPos), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(300, viewPortVerticalSize), ImGuiCond_Once);
            ImGui::SetNextWindowSizeConstraints(ImVec2(300, viewPortVerticalSize), 
                                                ImVec2(ImGui::GetIO().DisplaySize.x / 2.0f, viewPortVerticalSize));
            ImGui::Begin("Left Window", (bool*)NULL, ImGuiWindowFlags_NoCollapse 
                                                    | ImGuiWindowFlags_NoTitleBar 
                                                    | ImGuiWindowFlags_NoBringToFrontOnFocus);
                                                    
            // ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            // renderScene();
            // ImGui::PopStyleVar(1);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
            renderParams();
            ImGui::PopStyleVar(1);

            ImGui::End();
        
        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(4);

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(4);
}