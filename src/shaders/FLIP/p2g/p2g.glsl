#version 430 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

layout(std430, binding = 0) readonly buffer PartPosBuffer { vec4[] partPos; };
layout(std430, binding = 1) readonly buffer PartVelBuffer { vec4[] partVel; };
layout(std430, binding = 2) writeonly buffer RXBuffer { float[] rX; };
layout(std430, binding = 3) writeonly buffer RYBuffer { float[] rY; };
layout(std430, binding = 4) writeonly buffer RZBuffer { float[] rZ; };
layout(std430, binding = 5) writeonly buffer VelXBuffer { float[] velX; };
layout(std430, binding = 6) writeonly buffer VelYBuffer { float[] velY; };
layout(std430, binding = 7) writeonly buffer VelZBuffer { float[] velZ; };
layout(std430, binding = 8) writeonly buffer IsAirBuffer { uint[] isAir; };
layout(std430, binding = 9) readonly buffer FirstCellParticleBuffer { uint[] firstCellParticle; };
layout(std430, binding = 10) readonly buffer CellParticleIdsBuffer { uint[] cellParticleIds; };

uniform int partN;
uniform int gridX;
uniform int gridY;
uniform int gridZ;
uniform float h;

#pragma include "../utils.glsl"

void main(){
    int x = int(gl_GlobalInvocationID.x);
    int y = int(gl_GlobalInvocationID.y);
    int z = int(gl_GlobalInvocationID.z);
    if (x >= gridX || y >= gridY || z >= gridZ) return;

    ivec3 cellCoord = ivec3(x, y, z);
    int cell = coordToCell(cellCoord, gridX, gridY, gridZ);
    int cellX = coordToCell(cellCoord, gridX+1, gridY, gridZ);
    int cellY = coordToCell(cellCoord, gridX, gridY+1, gridZ);
    int cellZ = coordToCell(cellCoord, gridX, gridY, gridZ+1);
    vec3 cellCenter = coordToPos(cellCoord, gridX, gridY, gridZ);
    vec3 cellCenterX = cellCenter - vec3(h * 0.5, 0, 0);
    vec3 cellCenterY = cellCenter - vec3(0, h * 0.5, 0);
    vec3 cellCenterZ = cellCenter - vec3(0, 0, h * 0.5);
    
    float currentRX = 0;
    float currentRY = 0;
    float currentRZ = 0;
    float currentVelX = 0;
    float currentVelY = 0;
    float currentVelZ = 0;
    isAir[cell] = 1u;
    bool hasSetIsAirToFalse = false;

    for(int offsetX = -1; offsetX <= 1; offsetX++) {
        for(int offsetY = -1; offsetY <= 1; offsetY++) {
            for(int offsetZ = -1; offsetZ <= 1; offsetZ++)
            {
                int cx = x + offsetX;
                int cy = y + offsetY;
                int cz = z + offsetZ;
                if (cx < 0 || cx >= gridX || cy < 0 || cy >= gridY || cz < 0 || cz >= gridZ) continue;

                int newCell = coordToCell(ivec3(cx, cy, cz), gridX, gridY, gridZ);
                int rangeStart = int(firstCellParticle[newCell]);
                int rangeEnd = int(firstCellParticle[newCell+1]);

                for (int k = rangeStart; k < rangeEnd; k++)
                {
                    if (!hasSetIsAirToFalse && offsetX == 0 && offsetY == 0 && offsetZ == 0){
                        isAir[cell] = 0u;
                        hasSetIsAirToFalse = true;
                    }
                    int i = k;//int(cellParticleIds[k]);
                    vec3 pos = partPos[i].xyz;
                    vec3 vel = partVel[i].xyz;
                    {
                        vec3 dp = min(abs(pos - cellCenterX) / h, 1.0);
                        float w = (1.0 - dp.x) * (1.0 - dp.y) * (1.0 - dp.z);
                        
                        currentVelX += w * vel.x;
                        currentRX += w;
                    }
                    {
                        vec3 dp = min(abs(pos - cellCenterY) / h, 1.0);
                        float w = (1.0 - dp.x) * (1.0 - dp.y) * (1.0 - dp.z);
                        
                        currentVelY += w * vel.y;
                        currentRY += w;
                    }
                    {
                        vec3 dp = min(abs(pos - cellCenterZ) / h, 1.0);
                        float w = (1.0 - dp.x) * (1.0 - dp.y) * (1.0 - dp.z);
                        
                        currentVelZ += w * vel.z;
                        currentRZ += w;
                    }
                }
            }
        }
    }
    
    rX[cellX] = currentRX;
    rY[cellY] = currentRY;
    rZ[cellZ] = currentRZ;
    velX[cellX] = currentVelX;
    velY[cellY] = currentVelY;
    velZ[cellZ] = currentVelZ;
}