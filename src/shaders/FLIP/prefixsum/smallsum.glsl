#version 430 core
layout(local_size_x = 256) in;

shared uint temp[2048];
layout(std430, binding = 0) buffer DataBuffer { uint data[]; };

uniform int length;

const int ELEMENTS_PER_THREAD = 8;

void main(){
    uint tid = gl_LocalInvocationID.x;

    for (int k = 0; k < ELEMENTS_PER_THREAD; k++) {
        uint idx = tid * ELEMENTS_PER_THREAD + k;
        temp[idx] = (int(idx) < length) ? data[idx] : 0u;
    }

    barrier();

    if (tid == 0u){
        uint sum = 0u;
        for (int i = 0; i < length; i++) {
            uint val = temp[i];
            temp[i] = sum;
            sum += val;
        }
    }

    barrier();

    for (int k = 0; k < ELEMENTS_PER_THREAD; k++) {
        uint idx = tid * ELEMENTS_PER_THREAD + k;
        if (int(idx) < length) data[idx] = temp[idx];
    }
}