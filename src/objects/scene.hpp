#ifndef FLUIDGL_SCENE_HPP 
#define FLUIDGL_SCENE_HPP

#include <variant>
#include <functional>

#include "core/app.hpp"
#include "core/camera.hpp"
#include "solvers/solverManager.hpp"

using SolverConfig = std::variant<EulerSolverGPUConfig, LiquidFlipSolverGPUConfig, SmokeFlipSolverGPUConfig, FlipSolverCPUConfig, XpbdSolverCPUConfig>;

struct ObstacleInfo {
    std::string path = "";
    glm::vec3 pos = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(0.0f);
    bool is_static = true;
};

struct SceneInfo {
    SolverConfig solverConfig;
    std::shared_ptr<ObstacleInfo> obstacleInfo = {};
    std::function<void(std::shared_ptr<Mesh>, std::shared_ptr<App>, std::shared_ptr<Camera>)> updateMethod;
};

namespace Scene {
    inline SceneInfo suzanneCloseUpScene() {
        SceneInfo scene;
        LiquidFlipSolverGPUConfig flipConfig = LiquidFlipSolverGPUConfig();
        flipConfig.setPartPerH(2);
        flipConfig.setDensityMultiplier(1);
        flipConfig.setCgMaxIter(5);
        flipConfig.setCgTol(0.01f);
        flipConfig.setDomainSize(glm::vec3(226.0f, 158.0f, 218.5f));
        flipConfig.setPartRadius(0.5f);
        FluidInitializer initializer = FluidInitializer();
        initializer.setSizeAndPos(glm::vec3(120, 76, 96), glm::vec3(0, 29.5f, 0), flipConfig.getPartRadius());
        flipConfig.setFluidInitializer(initializer);
        flipConfig.setDt(0.07f);
        scene.solverConfig = flipConfig;

        scene.obstacleInfo = std::make_shared<ObstacleInfo>();
        scene.obstacleInfo->path = "models/suzanne.obj";
        scene.obstacleInfo->pos = glm::vec3(0.0f, -100.0f, -10.0f);
        scene.obstacleInfo->scale = glm::vec3(50.0f);
        scene.obstacleInfo->is_static = false;

        auto updateMethod = [dt = flipConfig.getDt()](std::shared_ptr<Mesh> suzanne, std::shared_ptr<App> app, std::shared_ptr<Camera> camera) {
            std::vector<float> ray = camera->rayFromMouse(app->mouseX(), app->mouseY(), app->width(), app->height());
            glm::vec3 rayOrigin = glm::vec3(ray[0], ray[1], ray[2]);
            glm::vec3 rayDir = glm::vec3(ray[3], ray[4], ray[5]);
            glm::vec3 pos = rayOrigin + glm::abs(rayOrigin.z) * rayDir / glm::max(glm::abs(rayDir.z), 1e-3f);

            glm::vec3 oldPos = suzanne->getPos();
            suzanne->setVel((pos - oldPos) / dt);
            suzanne->setPos(pos);
        };
        scene.updateMethod = updateMethod;

        return scene;
    }

    inline SceneInfo smokeTestScene() {
        SceneInfo scene;
        SmokeFlipSolverGPUConfig smokeConfig = SmokeFlipSolverGPUConfig();
        smokeConfig.setPartPerH(2);
        smokeConfig.setDensityMultiplier(1);
        smokeConfig.setCgMaxIter(5);
        smokeConfig.setCgTol(0.01f);
        smokeConfig.setPartRadius(0.8f);
        smokeConfig.setDomainSize(glm::vec3(175.0f, 250.0f, 175.0f));
        smokeConfig.setDt(0.07f);
        FluidInitializer densitySource = FluidInitializer();
        densitySource.setSizeAndPos(glm::vec3(40.0f), glm::vec3(0.0f, -95.0f, 0.0f), smokeConfig.getPartRadius());
        smokeConfig.setDensitySource(densitySource);
        smokeConfig.setDensityDecreaseRate(0.01f);
        scene.solverConfig = smokeConfig;

        scene.obstacleInfo = std::make_shared<ObstacleInfo>();
        scene.obstacleInfo->path = "models/suzanne.obj";
        scene.obstacleInfo->pos = glm::vec3(0.0f, -100.0f, -10.0f);
        scene.obstacleInfo->scale = glm::vec3(50.0f);
        scene.obstacleInfo->is_static = false;

        auto updateMethod = [dt = smokeConfig.getDt()](std::shared_ptr<Mesh> suzanne, std::shared_ptr<App> app, std::shared_ptr<Camera> camera) {
            std::vector<float> ray = camera->rayFromMouse(app->mouseX(), app->mouseY(), app->width(), app->height());
            glm::vec3 rayOrigin = glm::vec3(ray[0], ray[1], ray[2]);
            glm::vec3 rayDir = glm::vec3(ray[3], ray[4], ray[5]);
            glm::vec3 pos = rayOrigin + glm::abs(rayOrigin.z) * rayDir / glm::max(glm::abs(rayDir.z), 1e-3f);

            glm::vec3 oldPos = suzanne->getPos();
            suzanne->setVel((pos - oldPos) / dt);
            suzanne->setPos(pos);
        };
        scene.updateMethod = updateMethod;

        return scene;
    }

};
#endif