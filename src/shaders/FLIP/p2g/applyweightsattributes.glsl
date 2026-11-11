#version 430 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

layout(std430, binding = 0) readonly buffer WeightsBuffer { float[] weights; };

layout(r32f, binding = 0) uniform image3D gridAttributes;

uniform int gridX;
uniform int gridY;
uniform int gridZ;
uniform float h;

#pragma include "../utils.glsl"

void main(){
    int x = int(gl_GlobalInvocationID.x);
    int y = int(gl_GlobalInvocationID.y);
    int z = int(gl_GlobalInvocationID.z);
    if (x >= gridX || y >= gridY || z >= gridZ) return;

    ivec3 coord = ivec3(x, y, z);

    int i = coordToCell(coord, gridX, gridY, gridZ);
    float w = weights[i];
    if (w > 0){
        imageStore(gridAttributes, coord, vec4(imageLoad(gridAttributes, coord).x / w));
    }
}