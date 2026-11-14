#version 430 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

layout(std430, binding = 0) writeonly buffer PartPosBuffer { vec4 partPos[]; };
layout(std430, binding = 1) writeonly buffer PartVelBuffer { vec4 partVel[]; };
layout(std430, binding = 2) writeonly buffer DensityBuffer { float density[]; };
layout(std430, binding = 3) writeonly buffer TemperatureBuffer { float temperature[]; };
layout(std430, binding = 4) readonly buffer FirstCellParticleBuffer { uint firstCellParticle[]; };
layout(std430, binding = 5) buffer PartNBuffer { uint partN; };
layout(std430, binding = 6) writeonly buffer IsDeadBuffer { uint isDead[]; };
layout(std430, binding = 7) readonly buffer VelXBuffer { float velX[]; };
layout(std430, binding = 8) readonly buffer VelYBuffer { float velY[]; };
layout(std430, binding = 9) readonly buffer VelZBuffer { float velZ[]; };
layout(std430, binding = 10) readonly buffer IsAirBuffer { uint[] isAir; };

uniform int configPartN;
uniform uint frame;

uniform int gridX;
uniform int gridY;
uniform int gridZ;
uniform float h;
uniform float pph;

uniform vec3 minPos;

uniform bvec2 openBoundsX;
uniform bvec2 openBoundsY;
uniform bvec2 openBoundsZ;

#pragma include "../utils.glsl"
#pragma include "../velutils.glsl"

uvec3 pcg3d(uvec3 v) {
    v = v * 1664525u + 1013904223u;
    v.x += v.y * v.z;
    v.y += v.z * v.x;
    v.z += v.x * v.y;
    v ^= v >> 16u;
    v.x += v.y * v.z;
    v.y += v.z * v.x;
    v.z += v.x * v.y;
    return v;
}

vec3 rand3(uvec3 seed) {
    return vec3(pcg3d(seed)) * (1.0 / 4294967296.0);
}

vec3 randomInCell(ivec3 coord, uint id){
    uvec3 seed = uvec3(coord) ^ uvec3(id * 747796405u, frame * 2891336453u, id + frame);
    vec3 r = rand3(seed);
    return minPos + (vec3(coord) + r) * h;
}

void main(){
    int x = int(gl_GlobalInvocationID.x);
    int y = int(gl_GlobalInvocationID.y);
    int z = int(gl_GlobalInvocationID.z);
    bvec2 returnCondX = (openBoundsX && bvec2(x == 0, x == gridX-1)) || bvec2(x >= gridX);
    bvec2 returnCondY = (openBoundsY && bvec2(y == 0, y == gridY-1)) || bvec2(y >= gridY);
    bvec2 returnCondZ = (openBoundsZ && bvec2(z == 0, z == gridZ-1)) || bvec2(z >= gridZ);
    if (any(returnCondX) || any(returnCondY) || any(returnCondZ)) return;

    ivec3 coord = ivec3(x, y, z);
    int cell = coordToCell(coord, gridX, gridY, gridZ);

    uint numParticlesInCell = firstCellParticle[cell+1] - firstCellParticle[cell];
    uint targetPerCell = uint(floor(pph * pph * pph));
    if (numParticlesInCell >= 1) return;

    uint k = targetPerCell - numParticlesInCell;
    uint base = atomicAdd(partN, k);
    
    for(uint i = 0; i < k; i++){
        uint index = base + i;
        if (index >= configPartN) break;
        isDead[index] = 0u;
        vec3 newPos = randomInCell(coord, index);
        partPos[index] = vec4(newPos, 1.0);
        partVel[index] = vec4(gridVel(newPos, vec3(0.0)), 0.0);
        density[index] = 0.0;
        temperature[index] = 0.0;
    }

    atomicMin(partN, uint(configPartN));
}