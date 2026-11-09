#version 430 core
out vec4 FragColor;
in vec3 vClipPos;
in vec4 vColor;
in float depth;
in float radius;

uniform mat4 uProj;

void main()
{
    float dist2 = dot(vClipPos.xy, vClipPos.xy);
    if (dist2 > 1 || vColor.a < 0.05)
        discard;

    float z = sqrt(1.0 - dist2);
    float sphereDepth = depth - z * radius;
    
    vec4 clipZ = uProj * vec4(0.0, 0.0, -sphereDepth, 1.0);
    float ndcDepth = clipZ.z / clipZ.w;
    gl_FragDepth = ndcDepth * 0.5 + 0.5;

    FragColor = vec4(vec3(sphereDepth), 1);
    FragColor = vec4(vColor);
}