#version 430 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vVel;
layout (location = 2) in vec3 vNormal;

out vec3 vClipPos;
out vec3 normal;

uniform mat4 uView;
uniform mat4 uProj;

void main() {
    normal = vNormal;
    vClipPos = vPos;
    vec4 worldPos = vec4(vPos, 1.0);
    gl_Position = uProj * uView * worldPos;
}