#version 430 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

layout(std430, binding = 2) readonly buffer RXBuffer { float[] rX; };
layout(std430, binding = 3) readonly buffer RYBuffer { float[] rY; };
layout(std430, binding = 4) readonly buffer RZBuffer { float[] rZ; };
layout(std430, binding = 5) buffer VelXBuffer { float[] velX; };
layout(std430, binding = 6) buffer VelYBuffer { float[] velY; };
layout(std430, binding = 7) buffer VelZBuffer { float[] velZ; };
layout(std430, binding = 11) readonly buffer PartNBuffer { int partN; };

uniform int gridX;
uniform int gridY;
uniform int gridZ;
uniform float h;

#pragma include "../utils.glsl"

void main(){
    int x = int(gl_GlobalInvocationID.x);
    int y = int(gl_GlobalInvocationID.y);
    int z = int(gl_GlobalInvocationID.z);
    if (x > gridX || y > gridY || z > gridZ) return;

    ivec3 coord = ivec3(x, y, z);

    if (y < gridY && z < gridZ){
        int i = coordToCell(coord, gridX+1, gridY, gridZ);
        float r = rX[i];
        if (x == 0 || x == gridX) velX[i] = 0; // is wall
        else if (r > 0) velX[i] /= r;
    }
    if (x < gridX && z < gridZ){
        int i = coordToCell(coord, gridX, gridY+1, gridZ);
        float r = rY[i];
        if (y == 0 || y == gridY) velY[i] = 0; // is wall
        else if (r > 0) velY[i] /= r;
    }
    if (x < gridX && y < gridY){
        int i = coordToCell(coord, gridX, gridY, gridZ+1);
        float r = rZ[i];
        if (z == 0 || z == gridZ) velZ[i] = 0; // is wall
        else if (r > 0) velZ[i] /= r;
    }
}