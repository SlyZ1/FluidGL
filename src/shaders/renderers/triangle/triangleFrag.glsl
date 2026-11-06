#version 430 core

out vec4 FragColor;

in vec3 vClipPos;
in vec3 normal;

void main() {
    float lighting = dot(normal, normalize(vec3(2,3,1)));
    FragColor = vec4(vec3(lighting * 0.5 + 0.5), 1);
}