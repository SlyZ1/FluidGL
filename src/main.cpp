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
#include "objects/scene.hpp"

using namespace std;
using namespace glm;

int frameCount = 0;

shared_ptr<App> app;
shared_ptr<Camera> camera;
shared_ptr<UI> ui;

shared_ptr<SolverManager> solverManager = {};
shared_ptr<TriangleRenderer> triangleRenderer = {};
shared_ptr<Wireframes> obstacleWireframes = {};
WireframeIndex obstacleSDFBounds = 0;

SceneInfo sceneInfo = {};
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

void loadScene(SceneInfo scene){
    sceneInfo = scene;

    camera = make_shared<Camera>(app, 60.0f, 1200.f, 0.75f);
    camera->toggleFreeView(false);

    solverManager = make_shared<SolverManager>(camera);
    if (scene.obstacleInfo){

        shared_ptr<Mesh> obstacle = make_shared<Mesh>();
        obstacle->loadObj(scene.obstacleInfo->path);
        obstacle->setPos(scene.obstacleInfo->pos);
        obstacle->setScale(scene.obstacleInfo->scale);
        solverManager->setObstacle(obstacle);

        triangleRenderer = make_shared<TriangleRenderer>(camera);
        triangleRenderer->setModelMatrix(obstacle->getModelMatrix());
        triangleRenderer->setPosBuffer(obstacle->getPosBuffer());
        triangleRenderer->setNormalBuffer(obstacle->getNormalBuffer());
        triangleRenderer->setIndicies(obstacle->getIndiciesBuffer());

        obstacleWireframes = make_shared<Wireframes>(camera);
        obstacleSDFBounds = obstacleWireframes->addBox(obstacle->getSDFBounds().min, obstacle->getSDFBounds().max, vec4(0.5f, 0.5f, 0.5f, 1.0f));
        obstacleWireframes->uploadData();
    }

    visit([&](const auto& cfg) { solverManager->instantiate(cfg); }, scene.solverConfig);
}

void init(){
    app = make_shared<App>();
    app->init(1280, 720, "FluidGL");
    app->setIcon("public/FluidGL_Circle.png");
    app->setClearColor(0, 0, 0, 1.0f);
    app->toggleCursor(true);

    loadScene(Scene::suzanneCloseUpScene());

    vector<weak_ptr<IStatsProvider>> statsProviders = {};
    statsProviders.push_back(app);
    statsProviders.push_back(solverManager->getSolver());
    statsProviders.push_back(solverManager->getRenderer());
    if (sceneInfo.obstacleInfo) statsProviders.push_back(triangleRenderer);
    
    UIContext ctx = { app, solverManager };
    ui = make_shared<UI>(ctx);
    ui->setStatsContext(statsProviders);
    
    Logger::logSuccess("Program started.", __LOG_DATA__);
}

void inputs(){
    if (app->keyPressedOnce(GLFW_KEY_ESCAPE, frameCount)){
        camera->toggleFreeView(!camera->getFreeView());
        app->toggleCursor(!camera->getFreeView());
    }

    if (app->keyPressedOnce(GLFW_KEY_G, frameCount)){
        Wireframes::enableRender = !Wireframes::enableRender;
        UI::toggleRender = !UI::toggleRender;
    }

    if (app->keyPressedOnce(GLFW_KEY_R, frameCount)){
        solverManager->reloadRenderer();
        if (triangleRenderer) triangleRenderer->reload();
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

    triangleRenderer.reset();
    obstacleWireframes.reset();

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

        auto lockedObstacle = solverManager->getObstacle().lock();
        if (lockedObstacle) {
            if (sceneInfo.updateMethod) sceneInfo.updateMethod(lockedObstacle, app, camera);
            triangleRenderer->render();
            obstacleWireframes->render();
        }
        if (lockedObstacle && !sceneInfo.obstacleInfo->is_static) {
            obstacleWireframes->updateBox(obstacleSDFBounds, lockedObstacle->getSDFBounds().min, lockedObstacle->getSDFBounds().max, vec4(0.3f, 0.3f, 0.3f, 1.0f));
            obstacleWireframes->uploadData();
            triangleRenderer->setModelMatrix(lockedObstacle->getModelMatrix());
        }

        solverManager->update();
        solverManager->render();

        inputs();

        frameCount++;
        app->endFrame();
    }
    end();
    return EXIT_SUCCESS;
}