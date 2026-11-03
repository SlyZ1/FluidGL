#version 430 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

layout(std430, binding = 0) readonly buffer rBuffer { float r[]; };
layout(std430, binding = 1) writeonly buffer rCoarseBuffer { float rCoarse[]; };
layout(std430, binding = 2) readonly buffer IsAirBuffer { uint[] isAir; };
layout(std430, binding = 3) readonly buffer zBuffer { float z[]; };

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

    float childs = 0.0;
    for (int dx = 0; dx < f.x; dx++){
        for (int dy = 0; dy < f.y; dy++){
            for (int dz = 0; dz < f.z; dz++)
            {
                int I = i * f.x + dx; int J = j * f.y + dy; int K = k * f.z + dz;
                if (I >= gridX || J >= gridY || K >= gridZ) continue;

                int cell = coordToCell(ivec3(I, J, K), gridX, gridY, gridZ);
                if (isAir[cell] == 1u) continue;

                float s0 = float(I-1 >= 0);
                float s1 = float(I+1 < gridX);
                float s2 = float(J-1 >= 0);
                float s3 = float(J+1 < gridY);
                float s4 = float(K-1 >= 0);
                float s5 = float(K+1 < gridZ);
                float s = s0 + s1 + s2 + s3 + s4 + s5;

                if (s <= 0) continue;

                float zCell   = z[cell];
                float zLeft   = (s0 > 0.0) ? z[coordToCell(ivec3(I-1, J, K), gridX, gridY, gridZ)] : 0.0;
                float zRight  = (s1 > 0.0) ? z[coordToCell(ivec3(I+1, J, K), gridX, gridY, gridZ)] : 0.0;
                float zBottom = (s2 > 0.0) ? z[coordToCell(ivec3(I, J-1, K), gridX, gridY, gridZ)] : 0.0;
                float zTop    = (s3 > 0.0) ? z[coordToCell(ivec3(I, J+1, K), gridX, gridY, gridZ)] : 0.0;
                float zBack   = (s4 > 0.0) ? z[coordToCell(ivec3(I, J, K-1), gridX, gridY, gridZ)] : 0.0;
                float zFront  = (s5 > 0.0) ? z[coordToCell(ivec3(I, J, K+1), gridX, gridY, gridZ)] : 0.0;

                float az = s * zCell - (zLeft + zRight + zBottom + zTop + zBack + zFront);
                childs += r[cell] - az;
            }
        }
    }
    int coarseCell = coordToCell(ivec3(i,j,k), coarseGridX, coarseGridY, coarseGridZ);
    rCoarse[coarseCell] = childs * 0.25;
}