#include <iostream>
#include <fstream>
#include <sstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>

#include "core/app.hpp"
#include "core/shader_program.hpp"

#include "core/camera.hpp"

#include "helpers/stats.hpp"
#include "helpers/metrics.hpp"
#include "helpers/logger.hpp"

#include "solvers/solverManager.hpp"
#include "renderers/particleRenderer3D.hpp"

#include "ui/ui.hpp"

using namespace std;
using namespace glm;

int frameCount = 0;

shared_ptr<App> app;
shared_ptr<Camera> camera;
shared_ptr<IRenderer> renderer;
shared_ptr<UI> ui;

shared_ptr<SolverManager> solverManager = {};

vector<vec3> poses = { vec3(0,0,0), vec3(0.5f, 0.f, 0.f) };
int iterations = 1;

vec2 previousObstaclePos = vec2(0.f);
bool previousEnableObstacle = false;
bool enableObstacle = false;

#ifdef _WIN32
extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

void init(){
    app = make_shared<App>();
    app->init(1280, 720, "FluidGL");
    app->setIcon("public/FluidGL_Circle.png");
    app->setClearColor(0, 0, 0, 1.0f);
    app->toggleCursor(true);
    
    camera = make_shared<Camera>(app, 60.0f, 1200.f, 0.75f);
    camera->toggleFreeView(false);
    
    FlipSolverGPUConfig flipConfigGPU = FlipSolverGPUConfig();
    flipConfigGPU.setPartN((int)1e5);
    flipConfigGPU.setDt(0.05f / iterations);
    flipConfigGPU.setPartPerH(1.0f);
    flipConfigGPU.setDomainSize(vec3(600, 600, 300));
    flipConfigGPU.setPartRadius(1.5f);
    flipConfigGPU.setFluidInitializer({vec3(-100), vec3(100)});
    
    solverManager = make_shared<SolverManager>();
    weak_ptr<FlipSolverGPU> solver = solverManager->instantiate(flipConfigGPU);
    renderer = make_shared<ParticleRenderer3D>(*solverManager, camera);
    
    UIContext ctx = { app, solverManager, renderer };
    ui = make_shared<UI>(ctx);
    ui->setStatsContext({ app, solver });
    
    Logger::logSuccess("Program started.", __LOG_DATA__);
}

void inputs(shared_ptr<ISolver> lockedSolver){
    if (app->keyPressedOnce(GLFW_KEY_ESCAPE, frameCount)){
        camera->toggleFreeView(!camera->getFreeView());
        app->toggleCursor(!camera->getFreeView());
    }

    // Hot reload shaders
    if (app->keyPressedOnce(GLFW_KEY_R, frameCount)){
        if (renderer) renderer->reload();
        Logger::logInfo("Shaders reloaded.", __LOG_DATA__);
    }
    
    if (app->keyPressedOnce(GLFW_KEY_ENTER, frameCount)){
        if (lockedSolver) lockedSolver->reload();
        Logger::logInfo("Simulation restarted.", __LOG_DATA__);
    }

    if (app->keyPressedOnce(GLFW_KEY_P, frameCount)){
        lockedSolver->setPaused(!lockedSolver->isPaused());
        Logger::logInfo(lockedSolver->isPaused() ? "Simulation paused." : "Simulation resumed.", __LOG_DATA__);
    }

    if (app->keyPressedOnce(GLFW_KEY_RIGHT, frameCount)){
        for (int i = 0; i < iterations; i++)
            if (lockedSolver) lockedSolver->update();
    }
}

void end(){
    if (renderer) renderer.reset();
    solverManager.reset();

    ui.reset();
    camera.reset();

    app.reset();
}

int main(){
    init();
    while(!app->shouldClose())
    {
        app->startFrame(frameCount);
        ui->render();

        camera->update(app->dt() / 1000.0f);

        auto lockedSolver = solverManager->getSolver().lock();
        
        for (int i = 0; i < iterations; i++)
            if (lockedSolver) lockedSolver->update();

        renderer ? renderer->render() : void();

        inputs(lockedSolver);

        frameCount++;
        app->endFrame();
    }
    end();
    return EXIT_SUCCESS;
}