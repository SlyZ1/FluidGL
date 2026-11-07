#version 430 core
layout(local_size_x = 256) in;

layout(std430, binding = 0) buffer PartPosBuffer { vec4[] partPos; };
layout(std430, binding = 1) buffer PartVelBuffer { vec4[] partVel; };
layout(std430, binding = 2) buffer IsDeadBuffer { uint[] isDead; };
layout(std430, binding = 3) buffer PartNBuffer { uint partN; };
layout(std430, binding = 4) readonly buffer SDFBuffer { float obstacleSDF[]; };
layout(std430, binding = 5) readonly buffer SDFGradBuffer { vec4 obstacleSDFGrad[]; };

uniform int configPartN;

uniform vec3 minPos;
uniform vec3 maxPos;

uniform float radius;

uniform vec3 obstacleMinBound;
uniform vec3 obstacleMaxBound;
uniform vec3 obstacleVelocity;
uniform mat4 obstacleInverseMatrix;

uniform float h;
uniform ivec3 N;

#pragma include "./utils.glsl" 

void killParticle(int i){
    isDead[i] = 1u;
    atomicAdd(partN, -1u);
}

void main(){
    int i = int(gl_GlobalInvocationID.x);
    if (i >= configPartN) return;
    if (isDead[i] == 1u) return;

    vec3 pos = partPos[i].xyz;
    vec3 vel = partVel[i].xyz;
    // if (pos.y > maxPos.y - 4 * radius){
    //     partPos[i].x = maxPos.x + 100;
    //     killParticle(i);
    //     return;
    // }

    if (all(greaterThan(pos, obstacleMinBound)) && all(lessThan(pos, obstacleMaxBound))){
        float s = length(vec3(obstacleInverseMatrix[0]));
        vec3 posLocal = (obstacleInverseMatrix * vec4(pos, 1.0)).xyz;

        int cell = posToCell(posLocal, N.x, N.y, N.z);
        if (cell < N.x * N.y * N.z){
            float sdfVal = obstacleSDF[cell] / s;

            if (sdfVal <= 5 * radius) {
                vec3 normal = normalize(obstacleSDFGrad[cell].xyz);
                pos += normal * (5 * radius - sdfVal);

                vec3 relativeVel = vel - obstacleVelocity;
                relativeVel -= normal * dot(normal, relativeVel);
                vel = relativeVel + obstacleVelocity;
            }
        }
    }

    partPos[i] = vec4(clamp(pos, minPos + 1e-2f, maxPos - 1e-2f), 1);
    partVel[i] = vec4(vel, 1) * vec4(step(minPos, pos) * (1 - step(maxPos, pos)), 1);
}