#version 430 core

uniform sampler3D densityTexture;

uniform vec3 cameraPosition;
uniform vec3 domainSize;
uniform float h;

uniform float absorption;
uniform float lightAbsorption;
uniform float numStepSizeRatio;
uniform float numLightStepSizeRatio;
uniform float shadowMultiplier;

uniform vec3 lightDir;

uniform vec3 backgroundColor;

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
    float dCenter = texture(densityTexture, pCenter).r;
    return dCenter;
}

float beersLaw(float x, float absorption){
    return exp(-x*absorption);
}

float lightContrib(Ray ray, vec3 dir){
    float dx = h / numLightStepSizeRatio;
    float totalTransmittance = 1.0;

    while(isInside(ray.pos)){
        ray.pos += dir * dx;
        float d = sampleDensity(ray.pos);

        if (d > 0.001){
            float a = beersLaw(d * dx, lightAbsorption);
            totalTransmittance *= a;
        }
    }

    return totalTransmittance;
}

void main(){
    FragColor = vec4(vClipPos, 0.5);

    Ray ray;
    ray.dir = normalize(worldPos.xyz - cameraPosition);
    ray.pos = clamp(worldPos.xyz, -domainSize / 2 + vec3(1e-3), domainSize / 2 - vec3(1e-3));

    float dx = h / numStepSizeRatio;

    float totalTransmittance = 1.0;
    vec3 energy = vec3(0.0);

    while(isInside(ray.pos)){
        ray.pos += ray.dir * dx;
        float d = sampleDensity(ray.pos);

        if (d > 0.001){
            float a = beersLaw(d * dx, absorption);

            vec3 light = vec3(0.7) * mix(backgroundColor * shadowMultiplier, vec3(1), lightContrib(ray, lightDir));
            energy += totalTransmittance * (1 - a) * light;
            totalTransmittance *= a;
        }

    }

    vec3 finalColor = energy; //+ totalTransmittance * vec3(0.4,0.7,1.0);
    FragColor = vec4(energy, 1.0 - totalTransmittance);
}