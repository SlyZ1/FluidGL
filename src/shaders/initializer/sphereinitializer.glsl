#version 430 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

layout(std430, binding = 0) writeonly buffer GridBuffer { uint grid[]; };

uniform ivec3 gridDim;

void main() {
    int i = int(gl_GlobalInvocationID.x);
    int j = int(gl_GlobalInvocationID.y);
    int k = int(gl_GlobalInvocationID.z);
    ivec3 coord = ivec3(i, j, k);
    if (any(greaterThanEqual(coord, gridDim))) return;

    vec3 center = (vec3(gridDim) - 4) * 0.5;
    float radius = (gridDim.x - 4) * 0.5 - 1.0;

    grid[i + j * gridDim.x + k * gridDim.x * gridDim.y] = length(center - vec3(coord)) <= radius ? 1u : 0u;
}