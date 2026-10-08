#ifndef CAMERA_HPP
#define CAMERA_HPP
#include <glm/glm.hpp>

#include "core/app.hpp"

struct CameraProperties {
    float fov;
    float aperture;
    float focalLength;
};

class Camera {
    private:
        std::shared_ptr<App> m_app;

        float m_fov = 0;
        float m_moveSensitivity = 0;
        float m_lookSensitivity = 0;
        glm::vec3 m_pos = glm::vec3(0, 2, 1000);
        glm::vec2 m_angles = glm::vec2(0, 0);
        float m_lastMouseX = 0;
        float m_lastMouseY = 0;
        bool m_isMoving = false;
        bool m_isLooking = false;
        bool m_freeView = false;
        int m_lastMovingFrame = 0;
        static float s_targetZoom;
        float m_zoom = 1000.0f;

        CameraProperties m_camProps = { 50.0f, 0.0f, 1.0f };

        glm::vec4 keyboardInputs();
        void freeMove(float dt);
        void freeRotate(float deltaMouseX, float deltaMouseY, float dt);

        void lookAround(float deltaMouseX, float deltaMouseY, float dt);
        static void scrollCallback(double, double offsetY);
        void zoom(float dt);

    public:
        Camera(std::shared_ptr<App> app, float fov, float moveSensitivity, float lookSensitivity) 
            : m_app(app), m_fov(fov), m_moveSensitivity(moveSensitivity), m_lookSensitivity(lookSensitivity) {
                m_pos = glm::vec3(0.0f, 0.0f, 1000.0f);
                resetMousePos();
                m_app->addScrollCallback(scrollCallback);
            }

        void update(float dt);

        glm::vec3 getLookDirection() const;

        glm::vec3 getPosition() const { return m_pos; }
        void setPosition(glm::vec3 newPos) { m_pos = newPos; }

        bool getIsMoving(int frame);
        void hasStoppedMoving() { m_isMoving = false; m_isLooking = false; }
        void resetMousePos();

        void toggleFreeView(bool freeView);
        bool getFreeView() const { return m_freeView; }

        CameraProperties* getCameraProperties() { return &m_camProps; }

        float getFov() const {return m_fov;}
        void setFov(float fov) {m_fov = fov;}

        glm::mat4 viewMatrix() const;
        glm::mat4 projectionMatrix() const;
};

#endif