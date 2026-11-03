#version 430 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

layout(std430, binding = 0) readonly buffer IsAirBuffer { uint isAir[]; };
layout(std430, binding = 1) writeonly buffer IsAirCoarseBuffer { uint isAirCoarse[]; }; 

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

    if (i >= coarseGridX || j >= coarseGridY || k >= coarseGridZ) return;

    ivec3 coarseGrid = ivec3(coarseGridX, coarseGridY, coarseGridZ);
    ivec3 grid = ivec3(gridX, gridY, gridZ);
    ivec3 f = ivec3(1) + ivec3(lessThan(coarseGrid, grid));
    int coarseCell = coordToCell(ivec3(i,j,k), coarseGridX, coarseGridY, coarseGridZ);

    for (int dx = 0; dx < f.x; dx++){
        for (int dy = 0; dy < f.y; dy++){
            for (int dz = 0; dz < f.z; dz++)
            {
                int I = i * f.x + dx; int J = j * f.y + dy; int K = k * f.z + dz;
                if (I >= gridX || J >= gridY || K >= gridZ) continue;

                int cell = coordToCell(ivec3(I, J, K), gridX, gridY, gridZ);
                if (isAir[cell] == 0u){
                    isAirCoarse[coarseCell] = 0u;
                    return;
                }
            }
        }
    }
    isAirCoarse[coarseCell] = 1u;
}