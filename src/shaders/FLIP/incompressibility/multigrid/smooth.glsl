#version 430 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

layout(std430, binding = 0) readonly buffer rBuffer { float r[]; };
layout(std430, binding = 1) readonly buffer IsAirBuffer { uint[] isAir; };
layout(std430, binding = 2) buffer zBuffer { float z[]; };

uniform int color;
uniform int gridX;
uniform int gridY;
uniform int gridZ;


uniform float h; // unused

#pragma include "../../utils.glsl"

void main() {
    int i = int(gl_GlobalInvocationID.x);
    int j = int(gl_GlobalInvocationID.y);
    int k = int(gl_GlobalInvocationID.z);

    if (i >= gridX || j >= gridY || k >= gridZ) return;
    if ((i + j + k) % 2 == color) return;

    int cell = coordToCell(ivec3(i,j,k), gridX, gridY, gridZ);
    if (isAir[cell] == 1u) return;

    float s0 = float(i-1 >= 0);
    float s1 = float(i+1 < gridX);
    float s2 = float(j-1 >= 0);
    float s3 = float(j+1 < gridY);
    float s4 = float(k-1 >= 0);
    float s5 = float(k+1 < gridZ);
    float s = s0 + s1 + s2 + s3 + s4 + s5;

    if (s <= 0) return;

    float zLeft   = (s0 > 0.0) ? z[coordToCell(ivec3(i-1,j,k), gridX, gridY, gridZ)] : 0.0;
    float zRight  = (s1 > 0.0) ? z[coordToCell(ivec3(i+1,j,k), gridX, gridY, gridZ)] : 0.0;
    float zBottom = (s2 > 0.0) ? z[coordToCell(ivec3(i,j-1,k), gridX, gridY, gridZ)] : 0.0;
    float zTop    = (s3 > 0.0) ? z[coordToCell(ivec3(i,j+1,k), gridX, gridY, gridZ)] : 0.0;
    float zBack   = (s4 > 0.0) ? z[coordToCell(ivec3(i,j,k-1), gridX, gridY, gridZ)] : 0.0;
    float zFront  = (s5 > 0.0) ? z[coordToCell(ivec3(i,j,k+1), gridX, gridY, gridZ)] : 0.0;

    z[cell] = 1 / s * (r[cell] + zBottom + zTop + zLeft + zRight + zBack + zFront);
}