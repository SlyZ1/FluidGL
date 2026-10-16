#version 430 core
layout(local_size_x = 512) in;

layout(std430, binding = 0) readonly buffer CorrectionsBuffer { vec4[] corrections; };
layout(std430, binding = 1) readonly buffer NumCorrectionsBuffer { int[] numCorrections; };
layout(std430, binding = 4) buffer PartPosBuffer { vec4[] partPos; };
layout(std430, binding = 5) readonly buffer PartNBuffer { int partN; };


void main(){
    int i = int(gl_GlobalInvocationID.x);
    if (i >= partN) return;

    int num = numCorrections[i];
    if (num > 0) partPos[i].xyz += corrections[i].xyz / float(num);
}