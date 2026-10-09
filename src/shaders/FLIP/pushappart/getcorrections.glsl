#version 430 core
layout(local_size_x = 256) in;

layout(std430, binding = 0) readonly buffer FirstCellParticleBuffer { uint[] firstCellParticle; };
layout(std430, binding = 1) readonly buffer CellParticleIdsBuffer { uint[] cellParticleIds; };
layout(std430, binding = 2) readonly buffer OldPartPosBuffer { vec4[] oldPartPos; };
layout(std430, binding = 3) writeonly buffer PartPosBuffer { vec4[] partPos; };

uniform int partN;
uniform int gridX;
uniform int gridY;
uniform int gridZ;
uniform float h;

uniform float minDist;

#pragma include "../utils.glsl"

vec3 pseudoRandomDir(int i, int j) {
    // combine les deux indices de façon symétrique pour un hash cohérent
    uint seed = uint(min(i,j)) * 73856093u ^ uint(max(i,j)) * 19349663u;
    
    // génère 3 floats pseudo-aléatoires à partir du seed
    float x = fract(sin(float(seed) * 12.9898) * 43758.5453);
    float y = fract(sin(float(seed) * 78.233) * 43758.5453);
    float z = fract(sin(float(seed) * 37.719) * 43758.5453);
    
    vec3 dir = normalize(vec3(x, y, z) * 2.0 - 1.0); // remap [0,1] -> [-1,1], puis normalise
    
    // le signe dépend de si CE thread est i ou j, pour que les deux poussent en directions opposées
    return (i < j) ? dir : -dir;
}

void main(){
    int i = int(gl_GlobalInvocationID.x);
    if (i >= partN) return;

    vec3 currentPos = oldPartPos[i].xyz;
    ivec3 coord = posToCoord(currentPos, gridX, gridY, gridZ);
    vec3 cellCenter = coordToPos(coord, gridX, gridY, gridZ);
    
    bvec3 posCond = lessThan(currentPos, cellCenter);
    vec3 posCondInt = mix(vec3(0), vec3(1), posCond);

    vec3 currentCorrections = vec3(0.0);
    int currentNumCorrections = 0;

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
                    int j = k;//int(cellParticleIds[k]);
                    if (j == i) continue;

                    vec3 diff = currentPos - oldPartPos[j].xyz;
                    float dist2 = dot(diff, diff);
                    if (dist2 < minDist * minDist)
                    {
                        vec3 push = vec3(0);
                        const float correctionFactor = 0.5;
                        if (dist2 > 1e-9){
                            float dist = sqrt(dist2);
                            push = correctionFactor * diff * (minDist - dist) / dist;
                        }
                        else{
                            push = correctionFactor * minDist * pseudoRandomDir(i, j);
                        }
                        currentCorrections += push;
                        currentNumCorrections++;
                    }
                }
            }
        } 
    }

    if (currentNumCorrections > 0) 
        partPos[i] = vec4(currentPos + currentCorrections / currentNumCorrections, 1);
    else
        partPos[i] = vec4(currentPos, 1);
}