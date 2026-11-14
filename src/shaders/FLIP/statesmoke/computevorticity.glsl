#version 430 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

layout(std430, binding = 0) readonly buffer VelXBuffer { float velX[]; };
layout(std430, binding = 1) readonly buffer VelYBuffer { float velY[]; };
layout(std430, binding = 2) readonly buffer VelZBuffer { float velZ[]; };
layout(std430, binding = 3) writeonly buffer VorticityBuffer { vec4 vortiticy[]; };

uniform int gridX;
uniform int gridY;
uniform int gridZ;
uniform float h;

#pragma include "../utils.glsl"

float x(ivec3 coord) {
    coord = clamp(coord, ivec3(0), ivec3(gridX+1, gridY, gridZ));
    return 0.5 * (velX[coordToCell(coord, gridX+1, gridY, gridZ)] + velX[coordToCell(coord + ivec3(1,0,0), gridX+1, gridY, gridZ)]);
}
float y(ivec3 coord) {
    coord = clamp(coord, ivec3(0), ivec3(gridX, gridY+1, gridZ));
    return 0.5 * (velY[coordToCell(coord, gridX, gridY+1, gridZ)] + velY[coordToCell(coord + ivec3(0,1,0), gridX, gridY+1, gridZ)]);
}
float z(ivec3 coord) {
    coord = clamp(coord, ivec3(0), ivec3(gridX, gridY, gridZ+1));
    return 0.5 * (velZ[coordToCell(coord, gridX, gridY, gridZ+1)] + velZ[coordToCell(coord + ivec3(0,0,1), gridX, gridY, gridZ+1)]);
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
    float ax = 2.0 * float(i > 0 && i < gridX-1);
    float ay = 2.0 * float(j > 0 && j < gridY-1);
    float az = 2.0 * float(k > 0 && k < gridZ-1);
    vec3 w = 0.5 / h * vec3(
        ay * (z(coord+ey) - z(coord-ey)) - az * (y(coord+ez) - y(coord-ez)),
        az * (x(coord+ez) - x(coord-ez)) - ax * (z(coord+ex) - z(coord-ex)),
        ax * (y(coord+ex) - y(coord-ex)) - ay * (x(coord+ey) - x(coord-ey))
    );
    vortiticy[coordToCell(coord, gridX, gridY, gridZ)] = vec4(w, length(w));
}