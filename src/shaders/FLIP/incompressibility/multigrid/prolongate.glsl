#version 430 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

layout(std430, binding = 0) buffer zBuffer { float z[]; };
layout(std430, binding = 1) readonly buffer zCoarseBuffer { float zCoarse[]; };
layout(std430, binding = 2) readonly buffer IsAirBuffer { uint[] isAir; };

uniform int gridX;
uniform int gridY;
uniform int gridZ;
uniform int coarseGridX;
uniform int coarseGridY;
uniform int coarseGridZ;

uniform float h; // unused

#pragma include "../../utils.glsl"

void main() {
    int i = int(gl_GlobalInvocationID.x);
    int j = int(gl_GlobalInvocationID.y);
    int k = int(gl_GlobalInvocationID.z);
    if (i >= gridX || j >= gridY || k >= gridZ) return;

    int cell = coordToCell(ivec3(i, j, k), gridX, gridY, gridZ);
    if (isAir[cell] == 1u) return;
    
    ivec3 coarseDim = ivec3(coarseGridX, coarseGridY, coarseGridZ);
    ivec3 f = ivec3(1) + ivec3(lessThan(coarseDim, ivec3(gridX, gridY, gridZ)));
    ivec3 parent = ivec3(i, j, k) / f;
    int coarseCell = parent.x + coarseGridX * (parent.y + coarseGridY * parent.z);
    z[cell] += zCoarse[coarseCell];
}