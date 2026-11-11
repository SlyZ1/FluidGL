#version 430 core

uniform sampler3D densityTexture;

uniform vec3 cameraPosition;
uniform vec3 domainSize;
uniform float h;

uniform float absorption;
uniform float numStepSizeRatio;
uniform uint smoothDensity;

struct Ray {
    vec3 dir;
    vec3 pos;
};

in vec3 vClipPos;
in vec4 worldPos;
out vec4 FragColor;

bool isInside(vec3 p){
    return all(greaterThanEqual(p, -domainSize / 2)) && all(lessThanEqual(p, domainSize / 2));
}

float sampleDensity(vec3 p){
    vec3 pCenter = p / domainSize + 0.5;
    vec3 pLeft = (p + vec3(-h, 0, 0)) / domainSize + 0.5;
    vec3 pRight = (p + vec3(h, 0, 0)) / domainSize + 0.5;
    vec3 pDown = (p + vec3(0, -h, 0)) / domainSize + 0.5;
    vec3 pUp = (p + vec3(0, h, 0)) / domainSize + 0.5;
    vec3 pBack = (p + vec3(0, 0, -h)) / domainSize + 0.5;
    vec3 pFront = (p + vec3(0, 0, h)) / domainSize + 0.5;
    float dCenter = texture(densityTexture, pCenter).r;
    if (smoothDensity == 1u) return dCenter;
    float dLeft = texture(densityTexture, pLeft).r;
    float dRight = texture(densityTexture, pRight).r;
    float dDown = texture(densityTexture, pDown).r;
    float dUp = texture(densityTexture, pUp).r;
    float dBack = texture(densityTexture, pBack).r;
    float dFront = texture(densityTexture, pFront).r;
    return (dCenter * 2 + dLeft + dRight + dDown + dUp + dBack + dFront) / 8;
}

float beersLaw(float x, float absorption){
    return exp(-x*absorption);
}

void main(){
    FragColor = vec4(vClipPos, 0.5);

    Ray ray;
    ray.dir = normalize(worldPos.xyz - cameraPosition);
    ray.pos = clamp(worldPos.xyz, -domainSize / 2 + vec3(1e-3), domainSize / 2 - vec3(1e-3));

    float dx = h / numStepSizeRatio;

    float totalTransmittance = 1.0;
    float energy = 0.0;

    while(isInside(ray.pos)){
        ray.pos += ray.dir * dx;
        float d = sampleDensity(ray.pos);

        if (d > 0.001){
            float a = beersLaw(d * dx, absorption);

            energy += totalTransmittance * (1 - a);
            totalTransmittance *= a;
        }

    }

    FragColor = vec4(vec3(1), energy);
}