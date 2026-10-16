#version 430 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

layout(std430, binding = 0) readonly buffer XBuffer { float[] x; };
layout(std430, binding = 1) readonly buffer IsAirBuffer { uint[] isAir; };
layout(std430, binding = 2) writeonly buffer AXBuffer { float[] Ax; };

uniform int gridX;
uniform int gridY;
uniform int gridZ;
uniform float h;

#pragma include "../utils.glsl"

void main(){    
    int i = int(gl_GlobalInvocationID.x);
    int j = int(gl_GlobalInvocationID.y);
    int k = int(gl_GlobalInvocationID.z);

    if (i >= gridX || j >= gridY || k >= gridZ) return;

    int cell = coordToCell(ivec3(i,j,k), gridX, gridY, gridZ);
    if (isAir[cell] == 1u) 
        { Ax[cell] = 0.0; return; }

    float s0 = float(j-1 >= 0);
    float s1 = float(j+1 < gridY);
    float s2 = float(i-1 >= 0);
    float s3 = float(i+1 < gridX);
    float s4 = float(k-1 >= 0);
    float s5 = float(k+1 < gridZ);
    // float s0 = 0.0, s1 = 0.0, s2 = 0.0, s3 = 0.0, s4 = 0.0, s5 = 0.0;
    // if (j-1 >= 0) {
    //     int c = coordToCell(ivec3(i,j-1,k), gridX, gridY, gridZ);
    //     s0 = float(isAir[c] == 0u);
    // }
    // if (j+1 < gridY) {
    //     int c = coordToCell(ivec3(i,j+1,k), gridX, gridY, gridZ);
    //     s1 = float(isAir[c] == 0u);
    // }
    // if (i-1 >= 0) {
    //     int c = coordToCell(ivec3(i-1,j,k), gridX, gridY, gridZ);
    //     s2 = float(isAir[c] == 0u);
    // }
    // if (i+1 < gridX) {
    //     int c = coordToCell(ivec3(i+1,j,k), gridX, gridY, gridZ);
    //     s3 = float(isAir[c] == 0u);
    // }
    // if (k-1 >= 0) {
    //     int c = coordToCell(ivec3(i,j,k-1), gridX, gridY, gridZ);
    //     s4 = float(isAir[c] == 0u);
    // }
    // if (k+1 < gridZ) {
    //     int c = coordToCell(ivec3(i,j,k+1), gridX, gridY, gridZ);
    //     s5 = float(isAir[c] == 0u);
    // }
    float s = s0 + s1 + s2 + s3 + s4 + s5;
    const float eps = 1e-10;
    float xCell = x[cell];
    if (s <= 0.0) { Ax[cell] = eps * xCell; return; }

    float xBottom = (s0 > 0.0) ? x[coordToCell(ivec3(i,j-1,k), gridX, gridY, gridZ)] : 0.0;
    float xTop    = (s1 > 0.0) ? x[coordToCell(ivec3(i,j+1,k), gridX, gridY, gridZ)] : 0.0;
    float xLeft   = (s2 > 0.0) ? x[coordToCell(ivec3(i-1,j,k), gridX, gridY, gridZ)] : 0.0;
    float xRight  = (s3 > 0.0) ? x[coordToCell(ivec3(i+1,j,k), gridX, gridY, gridZ)] : 0.0;
    float xBack   = (s4 > 0.0) ? x[coordToCell(ivec3(i,j,k-1), gridX, gridY, gridZ)] : 0.0;
    float xFront  = (s5 > 0.0) ? x[coordToCell(ivec3(i,j,k+1), gridX, gridY, gridZ)] : 0.0;

    Ax[cell] = (s + eps) * xCell - (s0*xBottom + s1*xTop + s2*xLeft + s3*xRight + s4*xBack + s5*xFront);
}