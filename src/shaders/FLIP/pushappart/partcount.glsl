#version 430 core
layout(local_size_x = 64) in;

layout(std430, binding = 0) buffer NumCellParticlesBuffer { uint[] numCellParticles; };
layout(std430, binding = 1) writeonly buffer CellOfBuffer { uint[] cellOf; };
layout(std430, binding = 2) readonly buffer PartPosBuffer { vec4[] partPos; };
layout(std430, binding = 3) readonly buffer IsDeadBuffer { uint[] isDead; };

uniform int configPartN;

uniform int gridX;
uniform int gridY;
uniform int gridZ;
uniform float h;

#pragma include "../utils.glsl"

void main(){
    int i = int(gl_GlobalInvocationID.x);
    if (i >= configPartN) return;
    if (isDead[i] == 1u) return;

    int cell = posToCell(partPos[i].xyz, gridX, gridY, gridZ);
    cellOf[i] = uint(cell);
    atomicAdd(numCellParticles[cell], 1u);
}