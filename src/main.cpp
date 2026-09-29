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

bool freeView = false;

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
    app->toggleCursor(!freeView);
    
    camera = make_shared<Camera>(app, 60.0f, 0.02f, 1.5f);
    camera->resetMousePos(app->mouseX(), app->mouseY());
    
    FlipSolverGPUConfig flipConfigGPU = FlipSolverGPUConfig();
    flipConfigGPU.setPartN((int)1e5);
    flipConfigGPU.setDt(0.05f);
    flipConfigGPU.setPartPerH(2.0f);
    flipConfigGPU.setDomainSize(vec3(600, 600, 300));
    flipConfigGPU.setPartRadius(1.5f);
    
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
        freeView = !freeView;
        app->toggleCursor(!freeView);
        if (!freeView){
            camera->hasStoppedMoving();
        } else {
            camera->resetMousePos(app->mouseX(), app->mouseY());
        }
    }

    if (freeView){
        CameraMoveInputs inputs = {
            app->keyPressed(GLFW_KEY_W), 
            app->keyPressed(GLFW_KEY_S), 
            app->keyPressed(GLFW_KEY_D), 
            app->keyPressed(GLFW_KEY_A),
            app->keyPressed(GLFW_KEY_SPACE),
            app->keyPressed(GLFW_KEY_LEFT_CONTROL),
            app->keyPressed(GLFW_KEY_LEFT_SHIFT),
            app->keyPressed(GLFW_KEY_C)
        };
        camera->move(inputs, app->dt());
        camera->rotate(app->mouseX(), app->mouseY(), 1);
    };

    // Hot reload shaders
    if (app->keyPressedOnce(GLFW_KEY_R, frameCount)){
        if (renderer) renderer->reload();
        Logger::logInfo("Shaders reloaded.", __LOG_DATA__);
    }
    
    if (app->keyPressedOnce(GLFW_KEY_ENTER, frameCount)){
        if (lockedSolver) lockedSolver->reload();
        Logger::logInfo("Simulation restarted", __LOG_DATA__);
    }

    if (app->keyPressedOnce(GLFW_KEY_P, frameCount)){
        lockedSolver->setPaused(!lockedSolver->isPaused());
    }

    if (app->keyPressedOnce(GLFW_KEY_RIGHT, frameCount)){
        for (int i = 0; i < iterations; i++)
            if (lockedSolver) lockedSolver->update();
    }
    if (app->keyPressed(GLFW_MOUSE_BUTTON_LEFT)){
        enableObstacle = true;
    }
    else{
        enableObstacle = false;
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

        auto lockedSolver = solverManager->getSolver().lock();
        
        for (int i = 0; i < iterations; i++)
            if (lockedSolver) lockedSolver->update();

        if (renderer) {
            renderer->render();
        }

        inputs(lockedSolver);

        frameCount++;
        app->endFrame();
    }
    end();
    return EXIT_SUCCESS;
}