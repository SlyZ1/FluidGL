#version 430 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

struct Triangle {
    vec4 v1, n1;
    vec4 v2, n2;
    vec4 v3, n3;
};

layout(std430, binding = 0) writeonly buffer SDFBuffer { float sdf[]; };
layout(std430, binding = 1) readonly buffer TrianglesBuffer { Triangle triangles[]; };

uniform ivec3 N;
uniform int numTriangles;
uniform float dx;

#pragma include "./sdfutils.glsl"

vec3 closestPointOnTriangle(vec3 p, vec3 a, vec3 b, vec3 c)
{
    vec3 ab = b - a;
    vec3 ac = c - a;
    vec3 ap = p - a;

    // vertex A
    float d1 = dot(ab, ap);
    float d2 = dot(ac, ap);
    if (d1 <= 0.0 && d2 <= 0.0) return a;

    // vertex B
    vec3 bp = p - b;
    float d3 = dot(ab, bp);
    float d4 = dot(ac, bp);
    if (d3 >= 0.0 && d4 <= d3) return b;

    // edge AB
    float vc = d1 * d4 - d3 * d2;
    if (vc <= 0.0 && d1 >= 0.0 && d3 <= 0.0) {
        float v = d1 / (d1 - d3);
        return a + v * ab;
    }

    // vertex C
    vec3 cp = p - c;
    float d5 = dot(ab, cp);
    float d6 = dot(ac, cp);
    if (d6 >= 0.0 && d5 <= d6) return c;

    // edge AC
    float vb = d5 * d2 - d1 * d6;
    if (vb <= 0.0 && d2 >= 0.0 && d6 <= 0.0) {
        float w = d2 / (d2 - d6);
        return a + w * ac;
    }

    // edge BC
    float va = d3 * d6 - d5 * d4;
    if (va <= 0.0 && (d4 - d3) >= 0.0 && (d5 - d6) >= 0.0) {
        float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        return b + w * (c - b);
    }

    // face
    float denom = 1.0 / (va + vb + vc);
    float v = vb * denom;
    float w = vc * denom;
    return a + ab * v + ac * w;
}

vec3 bary(vec3 p, vec3 a, vec3 b, vec3 c) {
    vec3 v0 = b - a, v1 = c - a, v2 = p - a;
    float d00 = dot(v0,v0), d01 = dot(v0,v1), d11 = dot(v1,v1);
    float d20 = dot(v2,v0), d21 = dot(v2,v1);
    float den = d00*d11 - d01*d01;
    if (abs(den) < 1e-20) return vec3(1.0, 0.0, 0.0);
    float v = (d11*d20 - d01*d21) / den;
    float w = (d00*d21 - d01*d20) / den;
    return vec3(1.0 - v - w, v, w);
}

void main() {
    int i = int(gl_GlobalInvocationID.x);
    int j = int(gl_GlobalInvocationID.y);
    int k = int(gl_GlobalInvocationID.z);

    if (any(greaterThanEqual(ivec3(i, j, k), N))) return;

    vec3 p = coordToPos(i, j, k);
    float minDist2 = 1e15;
    float sdfSign = 1;
    for (int t = 0; t < numTriangles; t++){
        Triangle tri = triangles[t];

        vec3 closest = closestPointOnTriangle(p, tri.v1.xyz, tri.v2.xyz, tri.v3.xyz);
        vec3 d = p - closest;
        float dist2 = dot(d, d);

        if (minDist2 > dist2){
            minDist2 = dist2;

            vec3 bc = bary(closest, tri.v1.xyz, tri.v2.xyz, tri.v3.xyz);
            vec3 n = tri.n1.xyz * bc.x + tri.n2.xyz * bc.y + tri.n3.xyz * bc.z;

            sdfSign = dot(n, p - closest) >= 0 ? 1 : -1;
        }
    }

    sdf[coordToCell(i, j, k)] = sdfSign * sqrt(minDist2);
}