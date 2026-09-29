#ifndef UI_HPP
#define UI_HPP 

#include <type_traits>

#include "core/app.hpp"
#include "helpers/stats.hpp"
#include "renderers/renderer.hpp" 
#include "solverUI.hpp" 
#include "utilsUI.hpp" 
#include "solvers/solverManager.hpp"

struct UIContext {
    std::shared_ptr<App> app;
    std::weak_ptr<SolverManager> solverManager;
    std::weak_ptr<IRenderer> renderer;
};

class UI {
private:
    UIContext m_ctx = {};
    std::vector<std::weak_ptr<Stats>> m_statsCtx = {};
    SolverUI solverUI = {};

    // Useful functions
    static void TextWithShadow(
        const char* text,
        ImVec4 textColor = ImVec4(1,1,1,1),
        ImVec4 shadowColor = ImVec4(0.05f,0.05f,0.05f,0.7f),
        ImVec2 offset = ImVec2(1,1)
    );
    static void renderToolTip(const std::string& tip);

    static void AlignInputToRight(const char* input);

    static bool BeginCustomHeader(const std::string& name);
    static void EndCustomHeader();

    // Stats
    void drawTimers(StatsPrimitive<float> timers);
    void drawCounters(StatsPrimitive<int> counters);
    void drawStorages(StatsPrimitive<int> storage); 
    void renderStats();

    // Params
    template<typename T>
    void drawConfigField(T field, const std::string& name){
        UtilsUI::Label(name.c_str());
        if constexpr (std::is_same<T, int>::value) {
            ImGui::InputInt(("##" + name).c_str(), &field);
        } else if constexpr (std::is_same<T, float>::value){
            ImGui::DragFloat(("##" + name).c_str(), &field);
        }
    }
    void renderParams();

public:
    UI(UIContext ctx);
    ~UI() = default;
    void setStatsContext(const std::vector<std::weak_ptr<IStatsProvider>>& ctx);

    void render();
};

#endif