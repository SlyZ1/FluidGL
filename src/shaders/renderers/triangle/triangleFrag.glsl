#version 430 core

out vec4 FragColor;

in vec3 vClipPos;
in vec3 normal;

void main() {
    float lighting = dot(normal, normalize(vec3(1,1,1)) );
    FragColor = vec4(vClipPos, 1);
}