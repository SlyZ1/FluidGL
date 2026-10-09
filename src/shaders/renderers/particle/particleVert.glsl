#version 430 core
layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 cPos;
layout (location = 3) in vec3 cVel;
out vec3 vClipPos;
out vec4 vColor;
out float depth;
out float radius;

uniform float particleRadius;
uniform mat4 uView;
uniform mat4 uProj;

void main()
{
    vClipPos = vPos;
    vec4 worldPos = vec4(cPos, 1.0);
    //worldPos.z -= 1000;
    vec4 viewPos = uView * worldPos;

    float t = clamp(length(cVel) / 50.0, 0.0, 1.0);

    float customRadius = mix(particleRadius, particleRadius / 2, t) * 2;
    //customRadius = particleRadius * 4;
    radius = customRadius;
    viewPos.xy += vPos.xy * customRadius;
    gl_Position = uProj * viewPos;
    depth = -viewPos.z;

    vec4 blue = vec4(0.02, 0.04, 0.6, 1.0);
    vec4 blue2 = vec4(0.07, 0.55, 0.87, 0.0);
    vec4 white = vec4(0.51, 0.79, 1.0, 1.0);

    float edge = 0.7;

    float t1 = smoothstep(0.0, edge, t);
    float t2 = smoothstep(edge, 1.0, t);

    vec4 color = mix(
        mix(blue, blue2, t1),
        mix(blue2, white, t2),
        step(edge, t)
    );

    vColor = color;
}