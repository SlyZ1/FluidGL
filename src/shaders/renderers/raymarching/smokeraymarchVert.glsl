#version 430 core

layout (location = 0) in vec3 vPos;

uniform vec3 domainSize;

uniform mat4 uView;
uniform mat4 uProj;

out vec3 vClipPos;
out vec4 worldPos;

void main(){
    vClipPos = vPos;
    worldPos = vec4(vPos * domainSize / 2, 1.0);
    gl_Position = uProj * uView * worldPos;
}