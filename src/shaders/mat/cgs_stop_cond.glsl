#version 430 core
layout(local_size_x = 1) in;

layout(std430, binding = 0) buffer ScalarBuffer {
    float scalars[];
};
layout(std430, binding = 1) writeonly buffer IndirectBuffer {
    uint data[];
};

uniform int rTrIndex;
uniform int bTbIndex;
uniform int rTr0ValueIndex;
uniform int numScalars;
uniform int numIndirectParams;
uniform float tol;

void main(){
    float rTr = scalars[rTrIndex];
    float target = max(tol * tol * scalars[bTbIndex], 1e-30);

    if (!(rTr > target)){
        for(int i = 0; i < 3 * numIndirectParams; i++) {
            data[i] = 0;
        }
    }
    for(int i = 0; i < numScalars - 1; i++) {
        if (i == rTrIndex || i == bTbIndex) continue;
        scalars[i] = 0;
    }
}