#include "app.hpp"

#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_glfw.h>

#include "helpers/logger.hpp"
#include <unistd.h>
using namespace std;

App::App() : IStatsProvider("App") {
    for (int i = 0; i < GLFW_KEY_LAST + 1; i++)
        m_wasPressed[i] = INT_MAX;
}

App::~App(){
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void framebuffer_size_callback(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
}

void App::init(int width, int height, const char *name){
#ifdef __linux__
    // Prevent from mouse hidding issues 
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
#endif
    if (!glfwInit())
    {
        Logger::logError("Failed to initialize GLFW", __LOG_DATA__);
        exit(1);
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RED_BITS, 16);
    glfwWindowHint(GLFW_GREEN_BITS, 16);
    glfwWindowHint(GLFW_BLUE_BITS, 16);
    
    m_window = glfwCreateWindow(width, height, "ZMMR", NULL, NULL);
    glfwSetWindowTitle(m_window, name);
    if (m_window == NULL)
    {
        Logger::logError("Failed to open GLFW window", __LOG_DATA__);
        exit(1);
    }
    glfwMakeContextCurrent(m_window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        Logger::logError("Failed to initialize GLAD", __LOG_DATA__);
        exit(1);
    }

    glViewport(0, 0, width, height);
    glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    m_io = &ImGui::GetIO(); (void)(*m_io);
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 430");

    glfwSwapInterval(0);

    m_frameTimer.begin();
    m_fpsStatIndex = m_stats->registerCounter("FPS");
    m_frameTimeStatIndex = m_stats->registerTimer("Frame Time");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void App::setIcon(const char* path) {
    GLFWimage image[1];
    image[0].pixels = stbi_load(path, &image[0].width, &image[0].height, nullptr, STBI_rgb_alpha);
    if (!image[0].pixels) {
        Logger::logError("Failed to load icon: " + std::string(path) + " : " + stbi_failure_reason(), __LOG_DATA__);
        return;
    }
    glfwSetWindowIcon(m_window, 1, &image[0]);
    stbi_image_free((void*)image[0].pixels);
}

void App::setClearColor(float r, float g, float b, float a) const {
    glClearColor(r, g, b, a);
}

void App::startFrame(int frame) {
    if(keyPressedOnce(GLFW_KEY_Q, frame) && keyPressed(GLFW_KEY_LEFT_ALT))
        glfwSetWindowShouldClose(m_window, true);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    m_fpsCounter.update();
    m_frameTimer.end();
    m_frameTimer.begin();
    m_stats->setCounter(m_fpsStatIndex, m_fpsCounter.get());
    m_stats->setTimer(m_frameTimeStatIndex, m_frameTimer.get());

    double now = glfwGetTime();
    m_dt = (float)now - m_lastTime;
    m_lastTime = (float)now;
}

void App::endFrame() const {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

bool App::shouldClose() const {
    return glfwWindowShouldClose(m_window);
}

bool App::keyPressed(int key) const {
    return glfwGetKey(m_window, key) == GLFW_PRESS || glfwGetMouseButton(m_window, key) == GLFW_PRESS;
}

bool App::keyPressedOnce(int key, int frame) {
    bool isPressed = glfwGetKey(m_window, key) == GLFW_PRESS;

    if (!isPressed){
        m_wasPressed[key] = INT_MAX;
        return false;
    }
    if (m_wasPressed[key] < frame) return false;
    
    m_wasPressed[key] = frame;
    return true;
}

void App::toggleCursor(bool show){
    m_cursorHidden = !show;
    if (m_cursorHidden){
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        glfwPollEvents();
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    else{
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

bool App::cursorIsHidden() const {
    return m_cursorHidden;
}

float App::mouseX() const {
    double mouseX;
    glfwGetCursorPos(m_window, &mouseX, nullptr);
    float result = static_cast<float>(mouseX);
    return result;
}

float App::mouseY() const {
    double mouseY;
    glfwGetCursorPos(m_window, nullptr, &mouseY);
    float result = static_cast<float>(mouseY);
    return result;
}

float App::dt() const {
    return std::min(m_dt, 1.0f / 50.0f) * 1000.f;
}

unsigned int App::width() const {
    int width;
    glfwGetWindowSize(m_window, &width, nullptr);
    return width;
}

unsigned int App::height() const {
    int height;
    glfwGetWindowSize(m_window, nullptr, &height);
    return height;
}