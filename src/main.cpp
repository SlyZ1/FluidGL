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
shared_ptr<UI> ui;

shared_ptr<SolverManager> solverManager = {};

vector<vec3> poses = { vec3(0,0,0), vec3(0.5f, 0.f, 0.f) };

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
    
    FlipSolverGPUConfig flipConfig = FlipSolverGPUConfig();
    flipConfig.setPartN((int)1e5);
    flipConfig.setDt(0.05f);
    flipConfig.setPartPerH(1.0f);
    flipConfig.setDomainSize(vec3(600, 600, 300));
    flipConfig.setPartRadius(1.5f);
    flipConfig.setFluidInitializer({vec3(-100), vec3(100)});
    
    solverManager = make_shared<SolverManager>(camera);
    weak_ptr<FlipSolverGPU> solver = solverManager->instantiate(flipConfig);
    
    UIContext ctx = { app, solverManager };
    ui = make_shared<UI>(ctx);
    ui->setStatsContext({ app, solver });
    
    Logger::logSuccess("Program started.", __LOG_DATA__);
}

void inputs(){
    if (app->keyPressedOnce(GLFW_KEY_ESCAPE, frameCount)){
        camera->toggleFreeView(!camera->getFreeView());
        app->toggleCursor(!camera->getFreeView());
    }

    if (app->keyPressedOnce(GLFW_KEY_G, frameCount)){
        solverManager->toggleGizmos(!solverManager->getGizmosToggled());
    }

    if (app->keyPressedOnce(GLFW_KEY_R, frameCount)){
        solverManager->reloadRenderer();
        Logger::logInfo("Shaders reloaded.", __LOG_DATA__);
    }
    
    if (app->keyPressedOnce(GLFW_KEY_ENTER, frameCount)){
        solverManager->reloadSolver();
        Logger::logInfo("Simulation restarted.", __LOG_DATA__);
    }

    if (app->keyPressedOnce(GLFW_KEY_P, frameCount)){
        solverManager->togglePause();
    }

    if (app->keyPressedOnce(GLFW_KEY_RIGHT, frameCount)){
        solverManager->update();
    }
}

void end(){
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

        solverManager->update();
        solverManager->render();

        inputs();

        frameCount++;
        app->endFrame();
    }
    end();
    return EXIT_SUCCESS;
}