#include "camera.hpp"
#include <iostream>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

float Camera::s_targetZoom = 1000.0f;

vec3 Camera::getLookDirection() const {
    vec2 radAngles = vec2(radians(m_angles.x), radians(m_angles.y));
    vec3 lookDir = vec3(- sin(radAngles.x) * cos(radAngles.y), sin(radAngles.y), - cos(radAngles.x) * cos(radAngles.y));
    return normalize(lookDir);
}

vec4 Camera::keyboardInputs(){
    bool right = m_app->keyPressed(GLFW_KEY_RIGHT) || m_app->keyPressed(GLFW_KEY_D);
    bool left = m_app->keyPressed(GLFW_KEY_LEFT) || m_app->keyPressed(GLFW_KEY_A);
    bool forward = m_app->keyPressed(GLFW_KEY_UP) || m_app->keyPressed(GLFW_KEY_W);
    bool backward = m_app->keyPressed(GLFW_KEY_DOWN) || m_app->keyPressed(GLFW_KEY_S);
    bool up = m_app->keyPressed(GLFW_KEY_SPACE);
    bool down = m_app->keyPressed(GLFW_KEY_LEFT_CONTROL);
    bool sprinting = m_app->keyPressed(GLFW_KEY_LEFT_SHIFT);
    bool slowing = m_app->keyPressed(GLFW_KEY_C);
    return vec4(
        (int)right - (int)left, 
        (int)up - (int)down,
        (int)forward - (int)backward,
        ((int)sprinting + 1) / ((int)slowing * 5 + 1) 
    );
}

void Camera::freeMove(float dt){
    vec3 step = Utils::xyz(keyboardInputs());
    vec3 lookDirection = getLookDirection();
    vec3 moveForward = step.z * lookDirection;
    vec3 moveUp = vec3(0, step.y, 0);
    vec3 moveRight = step.x * normalize(cross(lookDirection, vec3(0,1,0)));
    m_isMoving = length(step) > 0;
    if (m_isMoving){
        float speedFactor = keyboardInputs().w * dt;
        m_pos += speedFactor * m_moveSensitivity * normalize(moveForward + moveUp + moveRight);
    }
}

void Camera::freeRotate(float deltaMouseX, float deltaMouseY, float) {
    m_angles += vec2(-deltaMouseX, -deltaMouseY) * m_lookSensitivity;
    m_angles.y = std::clamp(m_angles.y, -80.0f, 80.0f);
}

void Camera::lookAround(float deltaMouseX, float deltaMouseY, float) {
    m_angles += vec2(-deltaMouseX, -deltaMouseY) * m_lookSensitivity / 2.f;
    m_angles.y = std::clamp(m_angles.y, -80.0f, 80.0f);
    vec3 lookDirection = getLookDirection();
    m_pos = -lookDirection * m_zoom;
}

void Camera::scrollCallback(double, double offsetY) {
    s_targetZoom *= exp(-offsetY * 0.1f);
}

void Camera::zoom(float dt){
    m_zoom = mix(m_zoom, s_targetZoom, 10 * dt);
}

void Camera::update(float dt) {
    float mouseX = mix(m_lastMouseX, m_app->mouseX(), 1 - exp(-dt * 30));
    float mouseY = mix(m_lastMouseY, m_app->mouseY(), 1 - exp(-dt * 30));
    float deltaMouseX = (mouseX - m_lastMouseX);
    float deltaMouseY = (mouseY - m_lastMouseY);
    m_isLooking = length(vec2(-deltaMouseX, -deltaMouseY)) > 0;
    m_lastMouseX = mouseX;
    m_lastMouseY = mouseY;
    static bool mousePressed = false;

    if (m_freeView) {
        mousePressed = false;
        freeMove(dt);
        freeRotate(deltaMouseX, deltaMouseY, dt);
    }
    else {
        if (!mousePressed && m_app->keyPressed(GLFW_MOUSE_BUTTON_LEFT)){
            resetMousePos();
        }
        mousePressed = m_app->keyPressed(GLFW_MOUSE_BUTTON_LEFT); 

        zoom(dt);
        lookAround(mousePressed ? deltaMouseX : 0, mousePressed ? deltaMouseY : 0, dt);
    }
}



void Camera::resetMousePos(){
    m_lastMouseX = m_app->mouseX();
    m_lastMouseY = m_app->mouseY();
}

bool Camera::getIsMoving(int frame){
    bool result = m_isMoving || m_isLooking;
    if (result) m_lastMovingFrame = frame;
    return frame - m_lastMovingFrame < 10; 
}

void Camera::toggleFreeView(bool freeView) { 
    m_freeView = freeView; 
    m_zoom = length(m_pos);
    s_targetZoom = 1000.0f;
}

mat4 Camera::viewMatrix() const {
    vec3 forward = getLookDirection();
    vec3 worldUp = abs(forward.y) < 0.999f ? vec3(0,1,0) : vec3(0,0,1);
    return glm::lookAt(m_pos, m_pos + forward, worldUp);
}

mat4 Camera::projectionMatrix() const {
    return glm::perspective(radians(m_fov), (float)m_app->width() / m_app->height(), 0.1f, 20000.0f);
}