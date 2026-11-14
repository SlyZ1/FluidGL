#version 430 core
layout(local_size_x = 256) in;

layout(std430, binding = 0) readonly buffer PartPosBuffer { vec4 partPos[]; };
layout(std430, binding = 1) readonly buffer FirstCellParticleBuffer { uint firstCellParticle[]; };
layout(std430, binding = 2) writeonly buffer DensityBuffer { float density[]; };
layout(std430, binding = 3) writeonly buffer TemperatureBuffer { float temperature[]; };
layout(std430, binding = 4) readonly buffer OldDensityBuffer { float oldDensity[]; };
layout(std430, binding = 5) readonly buffer OldTemperatureBuffer { float oldTemperature[]; };
layout(std430, binding = 6) readonly buffer PartNBuffer { uint partN; };

uniform int gridX;
uniform int gridY;
uniform int gridZ;
uniform float h;
uniform float pph;

uniform float dt;

uniform float kappa;

#pragma include "../utils.glsl"

void main() {
    int i = int(gl_GlobalInvocationID.x);
    if (i >= partN) return;

    float currentDensity = oldDensity[i];
    float currentTemperature = oldTemperature[i];
    vec3 currentPos = partPos[i].xyz;
    ivec3 coord = posToCoord(currentPos, gridX, gridY, gridZ);

    float dDdt = 0;
    float dTdt = 0;

    float kernel = h;
    float kernel2 = kernel * kernel;

    for(int x = -1; x <= 1; x++) {
        for(int y = -1; y <= 1; y++) { 
            for(int z = -1; z <= 1; z++)
            {
                int cx = coord.x + x;
                int cy = coord.y + y;
                int cz = coord.z + z;
                if (cx < 0 || cx >= gridX || cy < 0 || cy >= gridY || cz < 0 || cz >= gridZ) continue;

                int newCell = coordToCell(ivec3(cx, cy, cz), gridX, gridY, gridZ);
                int rangeStart = int(firstCellParticle[newCell]);
                int rangeEnd = int(firstCellParticle[newCell+1]);


                for (int k = rangeStart; k < rangeEnd; k++)
                {
                    if (k == i) continue;

                    vec3 diff = currentPos - partPos[k].xyz;
                    float dist2 = dot(diff, diff);
                    if (dist2 > kernel2) continue;
                    
                    float distFactor = (kernel2 - dist2);
                    dDdt += distFactor * distFactor * (oldDensity[k] - currentDensity);
                    dTdt += distFactor * distFactor * (oldTemperature[k] - currentTemperature);
                }
            }
        } 
    }

    float sigma = 1.566681471 / pow(kernel, 9);
    float D = kappa * kernel2;
    float V = pow(h * pph, 3);
    float factor = 12 * D * sigma * V;

    density[i] = currentDensity + factor * dDdt * dt;
    temperature[i] = currentTemperature + factor * dTdt * dt;
}