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
#include "renderers/triangleRenderer3D.hpp"
#include "objects/mesh.hpp"

#include "ui/ui.hpp"

using namespace std;
using namespace glm;

int frameCount = 0;

shared_ptr<App> app;
shared_ptr<Camera> camera;
shared_ptr<UI> ui;

shared_ptr<SolverManager> solverManager = {};
shared_ptr<TriangleRenderer> triangleRenderer = {};
shared_ptr<Wireframes> suzanneWireframes = {};
bool toggleSuzanneWireframes = true;
weak_ptr<Mesh> suzanne;
WireframeIndex suzanneSDFBounds = 0;
float configDt = 1.0f;

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
    flipConfig.setPartPerH(2);
    flipConfig.setDensityMultiplier(1);
    flipConfig.setCgMaxIter(5);
    flipConfig.setCgTol(0.01f);
    flipConfig.setDomainSize(vec3(226.0f, 158.0f, 218.5f));
    flipConfig.setPartRadius(0.5f);
    FluidInitializer initializer = FluidInitializer();
    initializer.setSizeAndPos(vec3(120, 76, 96), vec3(0, 29.5f, 0), flipConfig.getPartRadius());
    flipConfig.setFluidInitializer(initializer);
    flipConfig.setDt(0.07f);
    configDt = flipConfig.getDt();

    solverManager = make_shared<SolverManager>(camera);
    weak_ptr<FlipSolverGPU> solver = solverManager->instantiate(flipConfig);
    
    suzanne = solverManager->getObstacle();
    auto lockedSuzanne = suzanne.lock();
    
    triangleRenderer = make_shared<TriangleRenderer>(camera);
    triangleRenderer->setModelMatrix(lockedSuzanne->getModelMatrix());

    suzanneWireframes = make_shared<Wireframes>(camera);
    suzanneSDFBounds = suzanneWireframes->addBox(lockedSuzanne->getSDFBounds().min, lockedSuzanne->getSDFBounds().max, vec4(0.5f, 0.5f, 0.5f, 1.0f));
    suzanneWireframes->uploadData();

    triangleRenderer->setPosBuffer(lockedSuzanne->getPosBuffer());
    triangleRenderer->setNormalBuffer(lockedSuzanne->getNormalBuffer());
    triangleRenderer->setIndicies(lockedSuzanne->getIndiciesBuffer());
    
    UIContext ctx = { app, solverManager };
    ui = make_shared<UI>(ctx);
    ui->setStatsContext({ app, solver, solverManager->getRenderer(), triangleRenderer });
    
    Logger::logSuccess("Program started.", __LOG_DATA__);
}

void inputs(){
    if (app->keyPressedOnce(GLFW_KEY_ESCAPE, frameCount)){
        camera->toggleFreeView(!camera->getFreeView());
        app->toggleCursor(!camera->getFreeView());
    }

    if (app->keyPressedOnce(GLFW_KEY_G, frameCount)){
        solverManager->toggleGizmos(!solverManager->getGizmosToggled());
        toggleSuzanneWireframes = !toggleSuzanneWireframes;
    }

    if (app->keyPressedOnce(GLFW_KEY_R, frameCount)){
        solverManager->reloadRenderer();
        triangleRenderer->reload();
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

    vector<float> ray = camera->rayFromMouse(app->mouseX(), app->mouseY(), app->width(), app->height());
    vec3 rayOrigin = vec3(ray[0], ray[1], ray[2]);
    vec3 rayDir = vec3(ray[3], ray[4], ray[5]);
    vec3 pos = rayOrigin + abs(rayOrigin.z) * rayDir / std::max(abs(rayDir.z), 1e-3f);

    auto lockedSuzanne = suzanne.lock();
    if (lockedSuzanne){
        vec3 oldPos = lockedSuzanne->getPos();
        lockedSuzanne->setVel((pos - oldPos) / configDt / 10.f);
        lockedSuzanne->setPos(pos);
        triangleRenderer->setModelMatrix(lockedSuzanne->getModelMatrix());
        suzanneWireframes->updateBox(suzanneSDFBounds, lockedSuzanne->getSDFBounds().min, lockedSuzanne->getSDFBounds().max, vec4(0.5f, 0.5f, 0.5f, 1.0f));
        suzanneWireframes->uploadData();
    }
}

void end(){
    solverManager.reset();

    triangleRenderer.reset();
    suzanneWireframes.reset();

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
        triangleRenderer->render();
        if (toggleSuzanneWireframes) suzanneWireframes->render();

        inputs();

        frameCount++;
        app->endFrame();
    }
    end();
    return EXIT_SUCCESS;
}