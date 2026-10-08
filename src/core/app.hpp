#ifndef APP_HPP
#define APP_HPP

#include <imgui/imgui.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <functional>

#include "helpers/metrics.hpp"
#include "helpers/stats.hpp"

class App : public IStatsProvider {
    private:
        GLFWwindow *m_window = {};
        ImGuiIO* m_io = {};
        bool m_cursorHidden = false;
        
        StatIndex m_fpsStatIndex = 0;
        StatIndex m_frameTimeStatIndex = 0;
        CPUTimer m_frameTimer = {};
        FPSCounter m_fpsCounter = {};

        static std::vector<std::function<void(double, double)>> s_scrollCallbacks;

        float m_lastTime = 0;
        float m_dt = 0;

        int m_wasPressed[GLFW_KEY_LAST + 1];

        static void callScrollCallbacks(GLFWwindow*, double offsetX, double offsetY);

    public:
        App();
        ~App() override;

        App(const App&) = delete;
        App& operator=(const App&) = delete;

        void init(int width, int height, const char *name);
        void setIcon(const char* path);
        void setClearColor(float r, float g, float b, float a) const ;
        void startFrame(int frameCount);
        void endFrame() const;
        bool shouldClose() const;
        bool keyPressed(int key) const;
        bool keyPressedOnce(int key, int frame);
        void toggleCursor(bool show);
        bool cursorIsHidden() const;
        float mouseX() const;
        float mouseY() const;
        void addScrollCallback(std::function<void(double, double)> callback);
        float dt() const;
        unsigned int width() const;
        unsigned int height() const;
};

#endif