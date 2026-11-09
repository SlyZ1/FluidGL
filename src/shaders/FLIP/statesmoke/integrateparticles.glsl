#version 430 core
layout(local_size_x = 256) in;

layout(std430, binding = 0) buffer PartPosBuffer { vec4[] partPos; };
layout(std430, binding = 1) buffer PartVelBuffer { vec4[] partVel; };
layout(std430, binding = 2) readonly buffer PartNBuffer { uint partN; };
layout(std430, binding = 3) buffer DensityBuffer { float density[]; };
layout(std430, binding = 4) buffer TemperatureBuffer { float temperature[]; };

uniform float dt;

uniform vec3 minPos;
uniform vec3 maxPos;

uniform vec3 minDensitySourcePos;
uniform vec3 maxDensitySourcePos;

uniform float flotAlpha;
uniform float flotBeta;
uniform float densityDecreaseRate;
uniform float temperatureDecreaseRate;

void main(){
    uint i = gl_GlobalInvocationID.x;
    if (i >= partN) return;

    vec3 pos = partPos[i].xyz;

    float d = density[i];
    float t = temperature[i];
    d = max(0.0, d * (1.0 - densityDecreaseRate * dt));
    t = max(0.0, t * (1.0 - temperatureDecreaseRate * t * dt));
    if (all(greaterThan(pos, minDensitySourcePos)) && all(lessThan(pos, maxDensitySourcePos))){
        d = 1;
        t = 1;
    }
    vec3 force = (-flotAlpha * d + flotBeta * t) * vec3(0, 1, 0);

    vec3 oldVel = partVel[i].xyz;
    vec3 newVel = oldVel + force * dt;

    partVel[i].xyz = newVel;
    partPos[i].xyz = pos + newVel * dt;
    density[i] = d;
    temperature[i] = t;
}