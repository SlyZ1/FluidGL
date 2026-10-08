#version 430 core
layout(local_size_x = 64) in;

layout(std430, binding = 0) readonly buffer PartPosBuffer { vec4[] partPos; };
layout(std430, binding = 1) buffer PartVelBuffer { vec4[] partVel; };
layout(std430, binding = 2) readonly buffer VelXBuffer { float[] velX; };
layout(std430, binding = 3) readonly buffer VelYBuffer { float[] velY; };
layout(std430, binding = 4) readonly buffer VelZBuffer { float[] velZ; };
layout(std430, binding = 5) readonly buffer OldVelXBuffer { float[] oldVelX; };
layout(std430, binding = 6) readonly buffer OldVelYBuffer { float[] oldVelY; };
layout(std430, binding = 7) readonly buffer OldVelZBuffer { float[] oldVelZ; };
layout(std430, binding = 8) readonly buffer IsAirBuffer { uint[] isAir; };

uniform int partN;
uniform int gridX;
uniform int gridY;
uniform int gridZ;
uniform float h;
uniform float flipRatio;

#pragma include "./utils.glsl"

void main(){
    int i = int(gl_GlobalInvocationID.x);
    if (i >= partN) return;

    vec3 pos = partPos[i].xyz;
    vec3 vel = partVel[i].xyz;
    {
        int cell = posToCell(pos - vec3(0., h * 0.5, h * 0.5), gridX, gridY, gridZ);
        ivec3 coord = cellToCoord(cell, gridX, gridY);
        vec3 cellCenter = coordToPos(coord, gridX, gridY, gridZ) - vec3(h * 0.5, 0., 0.);
        vec3 dp = (pos - cellCenter) / h;
        
        bool airBool100 = isAir[coordToCell(coord + ivec3(1,0,0), gridX, gridY, gridZ)] == 0u;
        bool airBool110 = isAir[coordToCell(coord + ivec3(1,1,0), gridX, gridY, gridZ)] == 0u;
        bool airBool101 = isAir[coordToCell(coord + ivec3(1,0,1), gridX, gridY, gridZ)] == 0u;
        bool airBool111 = isAir[coordToCell(coord + ivec3(1,1,1), gridX, gridY, gridZ)] == 0u;
        float s1 = float(isAir[coordToCell(coord + ivec3(0,0,0), gridX, gridY, gridZ)] == 0u || (coord.x + 1 <= gridX && airBool100));
        float s2 = float(airBool100 || (coord.x + 2 <= gridX && isAir[coordToCell(coord + ivec3(1+1,0,0), gridX, gridY, gridZ)] == 0u));
        float s3 = float(airBool110 || (coord.x + 2 <= gridX && isAir[coordToCell(coord + ivec3(1+1,1,0), gridX, gridY, gridZ)] == 0u));
        float s4 = float(isAir[coordToCell(coord + ivec3(0,1,0), gridX, gridY, gridZ)] == 0u || (coord.x + 1 <= gridX && airBool110));
        float s5 = float(isAir[coordToCell(coord + ivec3(0,0,1), gridX, gridY, gridZ)] == 0u || (coord.x + 1 <= gridX && airBool101));
        float s6 = float(airBool101 || (coord.x + 2 <= gridX && isAir[coordToCell(coord + ivec3(1+1,0,1), gridX, gridY, gridZ)] == 0u));
        float s7 = float(airBool111 || (coord.x + 2 <= gridX && isAir[coordToCell(coord + ivec3(1+1,1,1), gridX, gridY, gridZ)] == 0u));
        float s8 = float(isAir[coordToCell(coord + ivec3(0,1,1), gridX, gridY, gridZ)] == 0u || (coord.x + 1 <= gridX && airBool111));

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
            float flip = pic - (oldVelX[bottomLeft] * w1 + oldVelX[bottomRight] * w2 + oldVelX[topRight] * w3 + oldVelX[topLeft] * w4
                               + oldVelX[bottomLeftFront] * w5 + oldVelX[bottomRightFront] * w6 + oldVelX[topRightFront] * w7 + oldVelX[topLeftFront] * w8);
            pic /= sumW;
            flip /= sumW;
            vel.x += flip;
            vel.x = mix(pic, vel.x, flipRatio);
        }
    }
    {
        int cell = posToCell(pos - vec3(h * 0.5, 0., h * 0.5), gridX, gridY, gridZ);
        ivec3 coord = cellToCoord(cell, gridX, gridY);
        vec3 cellCenter = coordToPos(coord, gridX, gridY, gridZ) - vec3(0., h * 0.5, 0.);
        vec3 dp = (pos - cellCenter) / h;
        
        bool airBool010 = isAir[coordToCell(coord + ivec3(0,1,0), gridX, gridY, gridZ)] == 0u;
        bool airBool110 = isAir[coordToCell(coord + ivec3(1,1,0), gridX, gridY, gridZ)] == 0u;
        bool airBool011 = isAir[coordToCell(coord + ivec3(0,1,1), gridX, gridY, gridZ)] == 0u;
        bool airBool111 = isAir[coordToCell(coord + ivec3(1,1,1), gridX, gridY, gridZ)] == 0u;
        float s1 = float(isAir[coordToCell(coord + ivec3(0,0,0), gridX, gridY, gridZ)] == 0u || (coord.y + 1 <= gridY && airBool010));
        float s2 = float(isAir[coordToCell(coord + ivec3(1,0,0), gridX, gridY, gridZ)] == 0u || (coord.y + 1 <= gridY && airBool110));
        float s3 = float(airBool110 || (coord.y + 2 <= gridY && isAir[coordToCell(coord + ivec3(1,1+1,0), gridX, gridY, gridZ)] == 0u));
        float s4 = float(airBool010 || (coord.y + 2 <= gridY && isAir[coordToCell(coord + ivec3(0,1+1,0), gridX, gridY, gridZ)] == 0u));
        float s5 = float(isAir[coordToCell(coord + ivec3(0,0,1), gridX, gridY, gridZ)] == 0u || (coord.y + 1 <= gridY && airBool011));
        float s6 = float(isAir[coordToCell(coord + ivec3(1,0,1), gridX, gridY, gridZ)] == 0u || (coord.y + 1 <= gridY && airBool111));
        float s7 = float(airBool111 || (coord.y + 2 <= gridY && isAir[coordToCell(coord + ivec3(1,1+1,1), gridX, gridY, gridZ)] == 0u));
        float s8 = float(airBool011 || (coord.y + 2 <= gridY && isAir[coordToCell(coord + ivec3(0,1+1,1), gridX, gridY, gridZ)] == 0u));

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
            float flip = pic - (oldVelY[bottomLeft] * w1 + oldVelY[bottomRight] * w2 + oldVelY[topRight] * w3 + oldVelY[topLeft] * w4
                               + oldVelY[bottomLeftFront] * w5 + oldVelY[bottomRightFront] * w6 + oldVelY[topRightFront] * w7 + oldVelY[topLeftFront] * w8);
            pic /= sumW;
            flip /= sumW;
            vel.y += flip;
            vel.y = mix(pic, vel.y, flipRatio);
        }
    }
    {
        int cell = posToCell(pos - vec3(h * 0.5, h * 0.5, 0.), gridX, gridY, gridZ);
        ivec3 coord = cellToCoord(cell, gridX, gridY);
        vec3 cellCenter = coordToPos(coord, gridX, gridY, gridZ) - vec3(0., 0., h * 0.5);
        vec3 dp = (pos - cellCenter) / h;
        
        bool airBool001 = isAir[coordToCell(coord + ivec3(0,0,1), gridX, gridY, gridZ)] == 0u;
        bool airBool011 = isAir[coordToCell(coord + ivec3(0,1,1), gridX, gridY, gridZ)] == 0u;
        bool airBool101 = isAir[coordToCell(coord + ivec3(1,0,1), gridX, gridY, gridZ)] == 0u;
        bool airBool111 = isAir[coordToCell(coord + ivec3(1,1,1), gridX, gridY, gridZ)] == 0u;
        float s1 = float(isAir[coordToCell(coord + ivec3(0,0,0), gridX, gridY, gridZ)] == 0u || (coord.z + 1 <= gridZ && airBool001));
        float s2 = float(isAir[coordToCell(coord + ivec3(1,0,0), gridX, gridY, gridZ)] == 0u || (coord.z + 1 <= gridZ && isAir[coordToCell(coord + ivec3(1,0,0+1), gridX, gridY, gridZ)] == 0u));
        float s3 = float(isAir[coordToCell(coord + ivec3(1,1,0), gridX, gridY, gridZ)] == 0u || (coord.z + 1 <= gridZ && airBool111));
        float s4 = float(isAir[coordToCell(coord + ivec3(0,1,0), gridX, gridY, gridZ)] == 0u || (coord.z + 1 <= gridZ && airBool011));
        float s5 = float(airBool001 || (coord.z + 2 <= gridZ && isAir[coordToCell(coord + ivec3(0,0,1+1), gridX, gridY, gridZ)] == 0u));
        float s6 = float(isAir[coordToCell(coord + ivec3(1,0,1), gridX, gridY, gridZ)] == 0u || (coord.z + 2 <= gridZ && isAir[coordToCell(coord + ivec3(1,0,1+1), gridX, gridY, gridZ)] == 0u));
        float s7 = float(airBool111 || (coord.z + 2 <= gridZ && isAir[coordToCell(coord + ivec3(1,1,1+1), gridX, gridY, gridZ)] == 0u));
        float s8 = float(airBool011 || (coord.z + 2 <= gridZ && isAir[coordToCell(coord + ivec3(0,1,1+1), gridX, gridY, gridZ)] == 0u));

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
            float flip = pic - (oldVelZ[bottomLeft] * w1 + oldVelZ[bottomRight] * w2 + oldVelZ[topRight] * w3 + oldVelZ[topLeft] * w4
                               + oldVelZ[bottomLeftFront] * w5 + oldVelZ[bottomRightFront] * w6 + oldVelZ[topRightFront] * w7 + oldVelZ[topLeftFront] * w8);
            pic /= sumW;
            flip /= sumW;
            vel.z += flip;
            vel.z = mix(pic, vel.z, flipRatio);
        }
    }

    partVel[i].xyz = vel;
}