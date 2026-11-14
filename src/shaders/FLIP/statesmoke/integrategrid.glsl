#version 430 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;


layout(std430, binding = 0) buffer VelXBuffer { float velX[]; };
layout(std430, binding = 1) buffer VelYBuffer { float velY[]; };
layout(std430, binding = 2) buffer VelZBuffer { float velZ[]; };
layout(std430, binding = 3) readonly buffer VorticityForceBuffer { vec4 vorticityForce[]; };

uniform int gridX;
uniform int gridY;
uniform int gridZ;
uniform float h;

uniform float dt;

#pragma include "../utils.glsl"

void main() {
    int i = int(gl_GlobalInvocationID.x);
    int j = int(gl_GlobalInvocationID.y);
    int k = int(gl_GlobalInvocationID.z); 
    if (i == 0 || j == 0 || k == 0   ||   i >= gridX || j >= gridY || k >= gridZ)
        return;

    ivec3 coord = ivec3(i, j, k);
    int cell = coordToCell(coord, gridX, gridY, gridZ);
    int cellLeft = coordToCell(coord-ivec3(1,0,0), gridX, gridY, gridZ);
    int cellBottom = coordToCell(coord-ivec3(0,1,0), gridX, gridY, gridZ);
    int cellBack = coordToCell(coord-ivec3(0,0,1), gridX, gridY, gridZ);

    vec3 currentVorticityForce = vorticityForce[cell].xyz;

    velX[coordToCell(coord, gridX+1, gridY, gridZ)] += 0.5 * dt * (currentVorticityForce.x + vorticityForce[cellLeft].x);
    velY[coordToCell(coord, gridX, gridY+1, gridZ)] += 0.5 * dt * (currentVorticityForce.y + vorticityForce[cellBottom].y);
    velZ[coordToCell(coord, gridX, gridY, gridZ+1)] += 0.5 * dt * (currentVorticityForce.z + vorticityForce[cellBack].z);
}