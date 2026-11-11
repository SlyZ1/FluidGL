#version 430 core
layout(local_size_x = 256) in;

layout(std430, binding = 0) buffer PartPosBuffer { vec4[] partPos; };
layout(std430, binding = 1) buffer PartVelBuffer { vec4[] partVel; };
layout(std430, binding = 2) readonly buffer VelXBuffer { float velX[]; };
layout(std430, binding = 3) readonly buffer VelYBuffer { float velY[]; };
layout(std430, binding = 4) readonly buffer VelZBuffer { float velZ[]; };
layout(std430, binding = 5) readonly buffer IsAirBuffer { uint[] isAir; };
layout(std430, binding = 6) readonly buffer PartNBuffer { uint partN; };
layout(std430, binding = 7) buffer DensityBuffer { float density[]; };
layout(std430, binding = 8) buffer TemperatureBuffer { float temperature[]; };

uniform float dt;

uniform vec3 minPos;
uniform vec3 maxPos;

uniform vec3 minDensitySourcePos;
uniform vec3 maxDensitySourcePos;

uniform float flotAlpha;
uniform float flotBeta;
uniform float densityDecreaseRate;
uniform float temperatureDecreaseRate;

uniform int gridX;
uniform int gridY;
uniform int gridZ;
uniform float h;

#pragma include "../utils.glsl"

bool isFluidCell(ivec3 c){
    if (any(lessThan(c, ivec3(0))) || any(greaterThanEqual(c, ivec3(gridX, gridY, gridZ)))) return false;
    return isAir[c.x + gridX * (c.y + gridY * c.z)] == 0u;
}

bool faceActive(ivec3 f, int a){
    ivec3 e = ivec3(a == 0, a == 1, a == 2);
    return isFluidCell(f - e) || isFluidCell(f);
}

vec3 gridVel(vec3 pos, vec3 fallback){
    vec3 vel = fallback;
    {
        int cell = posToCell(pos - vec3(0., h * 0.5, h * 0.5), gridX, gridY, gridZ);
        ivec3 coord = cellToCoord(cell, gridX, gridY);
        vec3 cellCenter = coordToPos(coord, gridX, gridY, gridZ) - vec3(h * 0.5, 0., 0.);
        vec3 dp = (pos - cellCenter) / h;
        
        float s1 = float(faceActive(coord + ivec3(0,0,0), 0));
        float s2 = float(faceActive(coord + ivec3(1,0,0), 0));
        float s3 = float(faceActive(coord + ivec3(1,1,0), 0));
        float s4 = float(faceActive(coord + ivec3(0,1,0), 0));
        float s5 = float(faceActive(coord + ivec3(0,0,1), 0));
        float s6 = float(faceActive(coord + ivec3(1,0,1), 0));
        float s7 = float(faceActive(coord + ivec3(1,1,1), 0));
        float s8 = float(faceActive(coord + ivec3(0,1,1), 0));

        float w1 = s1 * (1. - dp.x) * (1. - dp.y) * (1. - dp.z);
        float w2 = s2 * dp.x * (1. - dp.y) * (1. - dp.z);
        float w3 = s3 * dp.x * dp.y * (1. - dp.z);
        float w4 = s4 * (1. - dp.x) * dp.y * (1. - dp.z);
        float w5 = s5 * (1. - dp.x) * (1. - dp.y) * dp.z;
        float w6 = s6 * dp.x * (1. - dp.y) * dp.z;
        float w7 = s7 * dp.x * dp.y * dp.z;
        float w8 = s8 * (1. - dp.x) * dp.y * dp.z;
        float sumW = w1 + w2 + w3 + w4 + w5 + w6 + w7 + w8;
        if (sumW > 0){
            int bottomLeft = coordToCell(coord + ivec3(0, 0, 0), gridX+1, gridY, gridZ);
            int bottomRight = coordToCell(coord + ivec3(1, 0, 0), gridX+1, gridY, gridZ);
            int topRight = coordToCell(coord + ivec3(1, 1, 0), gridX+1, gridY, gridZ);
            int topLeft = coordToCell(coord + ivec3(0, 1, 0), gridX+1, gridY, gridZ);
            int bottomLeftFront = coordToCell(coord + ivec3(0, 0, 1), gridX+1, gridY, gridZ);
            int bottomRightFront = coordToCell(coord + ivec3(1, 0, 1), gridX+1, gridY, gridZ);
            int topRightFront = coordToCell(coord + ivec3(1, 1, 1), gridX+1, gridY, gridZ);
            int topLeftFront = coordToCell(coord + ivec3(0, 1, 1), gridX+1, gridY, gridZ);

            float pic = velX[bottomLeft] * w1 + velX[bottomRight] * w2 + velX[topRight] * w3 + velX[topLeft] * w4
                    + velX[bottomLeftFront] * w5 + velX[bottomRightFront] * w6 + velX[topRightFront] * w7 + velX[topLeftFront] * w8;
            pic /= sumW;
            vel.x = pic;
        }
    }
    {
        int cell = posToCell(pos - vec3(h * 0.5, 0., h * 0.5), gridX, gridY, gridZ);
        ivec3 coord = cellToCoord(cell, gridX, gridY);
        vec3 cellCenter = coordToPos(coord, gridX, gridY, gridZ) - vec3(0., h * 0.5, 0.);
        vec3 dp = (pos - cellCenter) / h;
        
        float s1 = float(faceActive(coord + ivec3(0,0,0), 1));
        float s2 = float(faceActive(coord + ivec3(1,0,0), 1));
        float s3 = float(faceActive(coord + ivec3(1,1,0), 1));
        float s4 = float(faceActive(coord + ivec3(0,1,0), 1));
        float s5 = float(faceActive(coord + ivec3(0,0,1), 1));
        float s6 = float(faceActive(coord + ivec3(1,0,1), 1));
        float s7 = float(faceActive(coord + ivec3(1,1,1), 1));
        float s8 = float(faceActive(coord + ivec3(0,1,1), 1));

        float w1 = s1 * (1. - dp.x) * (1. - dp.y) * (1. - dp.z);
        float w2 = s2 * dp.x * (1. - dp.y) * (1. - dp.z);
        float w3 = s3 * dp.x * dp.y * (1. - dp.z);
        float w4 = s4 * (1. - dp.x) * dp.y * (1. - dp.z);
        float w5 = s5 * (1. - dp.x) * (1. - dp.y) * dp.z;
        float w6 = s6 * dp.x * (1. - dp.y) * dp.z;
        float w7 = s7 * dp.x * dp.y * dp.z;
        float w8 = s8 * (1. - dp.x) * dp.y * dp.z;
        float sumW = w1 + w2 + w3 + w4 + w5 + w6 + w7 + w8;
        if (sumW > 0){
            int bottomLeft = coordToCell(coord + ivec3(0, 0, 0), gridX, gridY+1, gridZ);
            int bottomRight = coordToCell(coord + ivec3(1, 0, 0), gridX, gridY+1, gridZ);
            int topRight = coordToCell(coord + ivec3(1, 1, 0), gridX, gridY+1, gridZ);
            int topLeft = coordToCell(coord + ivec3(0, 1, 0), gridX, gridY+1, gridZ);
            int bottomLeftFront = coordToCell(coord + ivec3(0, 0, 1), gridX, gridY+1, gridZ);
            int bottomRightFront = coordToCell(coord + ivec3(1, 0, 1), gridX, gridY+1, gridZ);
            int topRightFront = coordToCell(coord + ivec3(1, 1, 1), gridX, gridY+1, gridZ);
            int topLeftFront = coordToCell(coord + ivec3(0, 1, 1), gridX, gridY+1, gridZ);

            float pic = velY[bottomLeft] * w1 + velY[bottomRight] * w2 + velY[topRight] * w3 + velY[topLeft] * w4
                    + velY[bottomLeftFront] * w5 + velY[bottomRightFront] * w6 + velY[topRightFront] * w7 + velY[topLeftFront] * w8;
            pic /= sumW;
            vel.y = pic;
        }
    }
    {
        int cell = posToCell(pos - vec3(h * 0.5, h * 0.5, 0.), gridX, gridY, gridZ);
        ivec3 coord = cellToCoord(cell, gridX, gridY);
        vec3 cellCenter = coordToPos(coord, gridX, gridY, gridZ) - vec3(0., 0., h * 0.5);
        vec3 dp = (pos - cellCenter) / h;
        
        float s1 = float(faceActive(coord + ivec3(0,0,0), 2));
        float s2 = float(faceActive(coord + ivec3(1,0,0), 2));
        float s3 = float(faceActive(coord + ivec3(1,1,0), 2));
        float s4 = float(faceActive(coord + ivec3(0,1,0), 2));
        float s5 = float(faceActive(coord + ivec3(0,0,1), 2));
        float s6 = float(faceActive(coord + ivec3(1,0,1), 2));
        float s7 = float(faceActive(coord + ivec3(1,1,1), 2));
        float s8 = float(faceActive(coord + ivec3(0,1,1), 2));

        float w1 = s1 * (1. - dp.x) * (1. - dp.y) * (1. - dp.z);
        float w2 = s2 * dp.x * (1. - dp.y) * (1. - dp.z);
        float w3 = s3 * dp.x * dp.y * (1. - dp.z);
        float w4 = s4 * (1. - dp.x) * dp.y * (1. - dp.z);
        float w5 = s5 * (1. - dp.x) * (1. - dp.y) * dp.z;
        float w6 = s6 * dp.x * (1. - dp.y) * dp.z;
        float w7 = s7 * dp.x * dp.y * dp.z;
        float w8 = s8 * (1. - dp.x) * dp.y * dp.z;
        float sumW = w1 + w2 + w3 + w4 + w5 + w6 + w7 + w8;
        if (sumW > 0){
            int bottomLeft = coordToCell(coord + ivec3(0, 0, 0), gridX, gridY, gridZ+1);
            int bottomRight = coordToCell(coord + ivec3(1, 0, 0), gridX, gridY, gridZ+1);
            int topRight = coordToCell(coord + ivec3(1, 1, 0), gridX, gridY, gridZ+1);
            int topLeft = coordToCell(coord + ivec3(0, 1, 0), gridX, gridY, gridZ+1);
            int bottomLeftFront = coordToCell(coord + ivec3(0, 0, 1), gridX, gridY, gridZ+1);
            int bottomRightFront = coordToCell(coord + ivec3(1, 0, 1), gridX, gridY, gridZ+1);
            int topRightFront = coordToCell(coord + ivec3(1, 1, 1), gridX, gridY, gridZ+1);
            int topLeftFront = coordToCell(coord + ivec3(0, 1, 1), gridX, gridY, gridZ+1);

            float pic = velZ[bottomLeft] * w1 + velZ[bottomRight] * w2 + velZ[topRight] * w3 + velZ[topLeft] * w4
                    + velZ[bottomLeftFront] * w5 + velZ[bottomRightFront] * w6 + velZ[topRightFront] * w7 + velZ[topLeftFront] * w8;
            pic /= sumW;
            vel.z = pic;
        }
    }
    return vel;
}

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
    density[i] = d;
    temperature[i] = t;

    vec3 force = (-flotAlpha * d + flotBeta * t) * vec3(0, 1, 0);
    vec3 acc = force;

    vec3 oldVel = partVel[i].xyz;
    vec3 newVel = oldVel + acc * dt;
    partVel[i].xyz = newVel;

    // RK2
    vec3 posMid = clamp(pos + 0.5 * newVel * dt, minPos, maxPos);
    vec3 newPos = pos + (gridVel(posMid, newVel) + acc * dt) * dt;
    partPos[i].xyz = newPos;
}