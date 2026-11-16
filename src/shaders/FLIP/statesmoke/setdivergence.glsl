#version 430 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

layout(std430, binding = 0) writeonly buffer DivergenceBuffer { float divergence[]; };
layout(std430, binding = 1) readonly buffer SourceBuffer { uint source[]; };

uniform vec3 minSourceBounds;
uniform vec3 maxSourceBounds;
uniform ivec3 sourceGridDim;

uniform ivec3 gridDim;
uniform float h;

uniform float divergenceMag;

#pragma include "../utils.glsl"

void main() {
    int i = int(gl_GlobalInvocationID.x);
    int j = int(gl_GlobalInvocationID.y);
    int k = int(gl_GlobalInvocationID.z);
    ivec3 coord = ivec3(i, j, k);
    if (any(greaterThanEqual(coord, gridDim))) return;

    vec3 pos = coordToPos(coord, gridDim.x, gridDim.y, gridDim.z);
    if (any(lessThanEqual(pos, minSourceBounds)) || any(greaterThanEqual(pos, maxSourceBounds))) return;

    ivec3 sourceCoord = ivec3(floor((pos - minSourceBounds) / h));
    uint s = source[coordToCell(sourceCoord, sourceGridDim.x, sourceGridDim.y, sourceGridDim.z)];
    if (s == 0u) return;

    divergence[coordToCell(coord, gridDim.x, gridDim.y, gridDim.z)] = divergenceMag;
}