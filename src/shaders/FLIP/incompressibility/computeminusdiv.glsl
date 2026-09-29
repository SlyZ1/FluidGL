#version 430 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

layout(std430, binding = 0) readonly buffer VelXBuffer { float[] velX; };
layout(std430, binding = 1) readonly buffer VelYBuffer { float[] velY; };
layout(std430, binding = 2) readonly buffer VelZBuffer { float[] velZ; };
layout(std430, binding = 3) readonly buffer RhoBuffer { float[] rho; };
layout(std430, binding = 4) readonly buffer IsAirBuffer { uint[] isAir; };
layout(std430, binding = 5) writeonly buffer MinusDivBuffer { float[] minusDiv; };

uniform int gridX;
uniform int gridY;
uniform int gridZ;
uniform float h;
uniform float density;
uniform float densityMultiplier;

#pragma include "../utils.glsl"

void main(){
    int i = int(gl_GlobalInvocationID.x);
    int j = int(gl_GlobalInvocationID.y);
    int k = int(gl_GlobalInvocationID.z); 
    if (i >= gridX || j >= gridY || k >= gridZ) return;
        
    int cell = coordToCell(ivec3(i,j,k), gridX, gridY, gridZ);
    if (isAir[cell] == 1u) 
        { minusDiv[cell] = 0.0; return; }

    float s0 = float(j-1 >= 0);
    float s1 = float(j+1 < gridY);
    float s2 = float(i-1 >= 0);
    float s3 = float(i+1 < gridX);
    float s4 = float(k-1 >= 0);
    float s5 = float(k+1 < gridZ);
    float s = s0 + s1 + s2 + s3 + s4 + s5;
    if (s <= 0) 
        { minusDiv[cell] = 0.0; return; }

    int bottom = coordToCell(ivec3(i, j, k), gridX, gridY+1, gridZ);
    int top = coordToCell(ivec3(i, j+1, k), gridX, gridY+1, gridZ);
    int left = coordToCell(ivec3(i, j, k), gridX+1, gridY, gridZ);
    int right = coordToCell(ivec3(i+1, j, k), gridX+1, gridY, gridZ);
    int back = coordToCell(ivec3(i, j, k), gridX, gridY, gridZ+1);
    int front = coordToCell(ivec3(i, j, k+1), gridX, gridY, gridZ+1);
    
    float velBottom = velY[bottom];
    float velTop = velY[top];
    float velLeft = velX[left];
    float velRight = velX[right];
    float velBack = velZ[back];
    float velFront = velZ[front];

    float d = s1 * velTop - s0 * velBottom;
    d += s3 * velRight - s2 * velLeft;
    d += s5 * velFront - s4 * velBack;
    
    d -= max(rho[cell] - density * densityMultiplier, 0);

    minusDiv[cell] = -d;
}