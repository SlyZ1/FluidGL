#version 430 core
layout(local_size_x = 256) in;

layout(std430, binding = 0) writeonly buffer CorrectionsBuffer { vec4[] corrections; };
layout(std430, binding = 1) writeonly buffer NumCorrectionsBuffer { int[] numCorrections; };
layout(std430, binding = 2) readonly buffer PartNBuffer { int partN; };


void main(){
    int i = int(gl_GlobalInvocationID.x);
    if (i >= partN) return;

    corrections[i] = vec4(0);
    numCorrections[i] = 0;
}