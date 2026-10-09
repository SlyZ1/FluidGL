#version 430 core
layout(local_size_x = 256) in;

layout(std430, binding = 0) readonly buffer cellParticleIdsBuffer { uint cellParticleIds[]; };
layout(std430, binding = 1) readonly buffer PartPosBuffer { vec4 partPos[]; };
layout(std430, binding = 2) readonly buffer PartVelBuffer { vec4 partVel[]; };
layout(std430, binding = 3) writeonly buffer SortedPartPosBuffer { vec4 sortedPartPos[]; };
layout(std430, binding = 4) writeonly buffer SortedPartVelBuffer { vec4 sortedPartVel[]; };

uniform int partN;

void main() {
    uint k = gl_GlobalInvocationID.x;
    if (k >= partN) return;
    
    uint id = cellParticleIds[k];
    sortedPartPos[k] = partPos[id];
    sortedPartVel[k] = partVel[id];
}