#version 430 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

layout(std430, binding = 0) readonly buffer IsAirBuffer { uint[] isAir; };
layout(std430, binding = 1) writeonly buffer PressureBuffer { float[] pressure; };

uniform int gridX;
uniform int gridY;
uniform int gridZ;
uniform float h;

#pragma include "../utils.glsl"

void main(){
    int i = int(gl_GlobalInvocationID.x);
    int j = int(gl_GlobalInvocationID.y);
    int k = int(gl_GlobalInvocationID.z);
    if (i >= gridX || j >= gridY || k >= gridZ) return;

    int cell = coordToCell(ivec3(i,j,k), gridX, gridY, gridZ);
    if (isAir[cell] == 1u) pressure[cell] = 0;
}