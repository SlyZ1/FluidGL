#version 430 core
layout(local_size_x = 256) in;

layout(std430, binding = 0) writeonly buffer Data { float data[]; };

uniform float value;
uniform int N;

void main(){
    uint i = gl_GlobalInvocationID.x;
    if (i >= N) return;

    data[i] = value;
}