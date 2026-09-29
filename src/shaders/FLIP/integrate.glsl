#version 430 core
layout(local_size_x = 256) in;

layout(std430, binding = 0) buffer PartPosBuffer { vec4[] partPos; };
layout(std430, binding = 1) buffer PartVelBuffer { vec4[] partVel; };

uniform float dt;
uniform int partN;

uniform vec3 minPos;
uniform vec3 maxPos;

uniform float gravity;

void main(){
    uint i = gl_GlobalInvocationID.x;
    if (i >= partN) return;

    vec3 acc = vec3(0, -gravity, 0);
    vec3 oldVel = partVel[i].xyz;
    vec3 newVel = oldVel + acc * dt;
    vec3 pos = partPos[i].xyz;

    if (any(greaterThan(pos, maxPos)) || any(lessThan(pos, minPos))){
        //newVel -= oldVel * 0.3 * dt;
    }

    partVel[i].xyz = newVel;
    partPos[i].xyz += newVel * dt;
}