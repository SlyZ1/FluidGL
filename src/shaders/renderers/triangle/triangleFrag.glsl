#version 430 core

uniform vec3 backgroundColor;

out vec4 FragColor;

in vec3 vClipPos;
in vec3 normal;

void main() {
    float lighting = dot(normal, normalize(vec3(2,3,1)));
    lighting = lighting * 0.5 + 0.5;
    vec3 color = mix(backgroundColor * 0.7, vec3(1), lighting);

    FragColor = vec4(color, 1);
}