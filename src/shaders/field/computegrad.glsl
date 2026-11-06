#version 430 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

layout(std430, binding = 0) readonly buffer DataBuffer { float data[]; };
layout(std430, binding = 1) writeonly buffer GradBuffer { vec4 grad[]; };

uniform ivec3 N;
uniform float h;

int coordToCell(ivec3 coord) {
    return coord.x + coord.y * N.x + coord.z * N.x * N.y;
}

void main() {
    ivec3 coord = ivec3(gl_GlobalInvocationID);
    if (any(greaterThanEqual(coord, N))) return;

    vec3 gradValue;
    for (int axis = 0; axis < 3; axis++) {
        ivec3 low = coord;
        ivec3 high = coord;
        low[axis] = max(coord[axis] - 1, 0);
        high[axis] = min(coord[axis] + 1, N[axis] - 1);

        float span = float(high[axis] - low[axis]) * h;
        gradValue[axis] = (data[coordToCell(high)] - data[coordToCell(low)]) / span;
    }

    grad[coordToCell(coord)] = vec4(gradValue, 0.0);
}