#version 430 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

layout(std430, binding = 0) readonly buffer PartPosBuffer { vec4[] partPos; };
layout(std430, binding = 1) readonly buffer AttributesBuffer { float[] attributes; };
layout(std430, binding = 2) readonly buffer IsAirBuffer { uint[] isAir; };
layout(std430, binding = 3) readonly buffer FirstCellParticleBuffer { uint[] firstCellParticle; };

layout(r32f, binding = 0) uniform writeonly image3D gridAttributes;

uniform int gridX;
uniform int gridY;
uniform int gridZ;
uniform float h;

#pragma include "../utils.glsl"

void main() {
    int x = int(gl_GlobalInvocationID.x);
    int y = int(gl_GlobalInvocationID.y);
    int z = int(gl_GlobalInvocationID.z);
    if (x >= gridX || y >= gridY || z >= gridZ) return;
    
    ivec3 cellCoord = ivec3(x, y, z);
    int cell = coordToCell(cellCoord, gridX, gridY, gridZ);
    vec3 cellCenter = coordToPos(cellCoord, gridX, gridY, gridZ);
    
    float currentW = 0;
    float currentGridAttributes = 0;

    bool currentIsAir = isAir[cell] == 1u;

    for(int offsetX = -1; offsetX <= 1; offsetX++) {
        for(int offsetY = -1; offsetY <= 1; offsetY++) {
            for(int offsetZ = -1; offsetZ <= 1; offsetZ++)
            {
                int cx = x + offsetX;
                int cy = y + offsetY;
                int cz = z + offsetZ;
                if (cx < 0 || cx >= gridX || cy < 0 || cy >= gridY || cz < 0 || cz >= gridZ) continue;

                int newCell = coordToCell(ivec3(cx, cy, cz), gridX, gridY, gridZ);

                if (currentIsAir && isAir[newCell] == 1u) continue;

                int rangeStart = int(firstCellParticle[newCell]);
                int rangeEnd = int(firstCellParticle[newCell+1]);

                for (int k = rangeStart; k < rangeEnd; k++)
                {
                    vec3 pos = partPos[k].xyz;
                    float att = attributes[k];
                    
                    vec3 dp = min(abs(pos - cellCenter) / h, 1.0);
                    float w = (1.0 - dp.x) * (1.0 - dp.y) * (1.0 - dp.z);

                    currentGridAttributes += w * att;
                    currentW += w;
                }
            }
        }
    }
    
    if (currentW > 0) imageStore(gridAttributes, cellCoord, vec4(currentGridAttributes / currentW));
}