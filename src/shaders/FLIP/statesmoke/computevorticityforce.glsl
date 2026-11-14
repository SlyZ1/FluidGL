#version 430 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

layout(std430, binding = 0) readonly buffer VorticityBuffer { vec4 vortiticy[]; };
layout(std430, binding = 1) writeonly buffer ForceBuffer { vec4 force[]; };

uniform int gridX;
uniform int gridY;
uniform int gridZ;
uniform float h;

uniform float eps;

#pragma include "../utils.glsl"

float wLength(ivec3 coord) {
    coord = clamp(coord, ivec3(0), ivec3(gridX, gridY, gridZ));
    return vortiticy[coordToCell(coord, gridX, gridY, gridZ)].w;
}

void main() {
    int i = int(gl_GlobalInvocationID.x);
    int j = int(gl_GlobalInvocationID.y);
    int k = int(gl_GlobalInvocationID.z); 
    if (i >= gridX || j >= gridY || k >= gridZ) return;

    ivec3 coord = ivec3(i, j, k);
    ivec3 ex = ivec3(1, 0, 0);
    ivec3 ey = ivec3(0, 1, 0);
    ivec3 ez = ivec3(0, 0, 1);
    float ax = 2 * float(i > 0 && i < gridX-1);
    float ay = 2 * float(j > 0 && j < gridY-1);
    float az = 2 * float(k > 0 && k < gridZ-1);

    vec3 gradW = 0.5 / h * vec3(
        ax * (wLength(coord+ex) - wLength(coord-ex)),
        ay * (wLength(coord+ey) - wLength(coord-ey)),
        az * (wLength(coord+ez) - wLength(coord-ez))
    );
    gradW /= (length(gradW) + 1e-5);

    vec3 f = eps * h * cross(gradW, vortiticy[coordToCell(coord, gridX, gridY, gridZ)].xyz);
    force[coordToCell(coord, gridX, gridY, gridZ)] = vec4(f, 0.0);
}