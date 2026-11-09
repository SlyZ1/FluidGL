#version 430 core
layout(local_size_x = 256) in;

layout(std430, binding = 0) readonly buffer cellParticleIdsBuffer { uint cellParticleIds[]; };
layout(std430, binding = 1) readonly buffer AttributesBuffer { float attributes[]; };
layout(std430, binding = 2) writeonly buffer SortedAttributesBuffer { float sortedAttributes[]; };
layout(std430, binding = 3) readonly buffer PartNBuffer { int partN; };

uniform int configPartN;

void main() {
    uint k = gl_GlobalInvocationID.x;
    if (k >= configPartN) return;

    bool alive = k < partN;
    if (!alive) return;
    
    uint id = cellParticleIds[k];
    sortedAttributes[k] = attributes[id];
}