
layout(push_constant) uniform UniformBufferObject{
    mat4 viewProj;
    vec3 pos;
    float timeSinceStart;
    uint asteroidLod1StartIndex;
    uint asteroidLod2StartIndex;
    uint asteroidTotalIndices;
} ubo;

struct ArgumentEntry{
    uint drawId;
};

#define PI 3.1415926538

float rand(in vec2 ip) {
    const float seed = 12345678;
    uvec2 p = uvec2(floatBitsToUint(ip.x), floatBitsToUint(ip.y));
    uint s = floatBitsToUint(seed);
    s ^= (s ^ ((~p.x << s) + (~p.y << s)));
    
    p ^= (p << 17U);
    p ^= ((p ^ s) >> 13U);
    p ^= (p << 5U);
    p ^= ((s + (s&(p.x^p.y))) >> 3U);
    
    uint n = (p.x*s+p.y)+((p.x ^ p.y) << ~p.x ^ s) + ((p.x ^ p.y) << ~p.y ^ s);
    return float(n*50323U) / float(0xFFFFFFFFU);
}

vec3 calcBaseLighting(vec3 inNormal){
    const vec3 toLight = vec3(0,0,1);
    const float intensity = 1;
    
    float ndotl = max(dot(inNormal,toLight), 0);
    
    vec3 color = vec3(1,1,1);
    vec3 diffuse = color * ndotl;
    vec3 ambientlight = vec3(0.1,0.1,0.1) * color;
    
    return (intensity * diffuse) + ambientlight;
}

vec3 polar2cartesian(float xangle, float zangle){
    return vec3(
                sin(zangle) * cos(xangle),
                sin(zangle) * sin(xangle),
                cos(zangle)
                );
}

vec3 genAsteroidInitialPosition(uint pos){
    const float maxDist = 100;
    const float minDist = 20;
    
    vec3 randDir = polar2cartesian(rand(vec2(pos,-int(pos))) * 2*PI, rand(vec2(pos+1,-int(pos))) * 2 * PI);
    
    randDir = normalize(randDir);
    
    float randDistance = mix(minDist, maxDist, rand(vec2(randDir.x,randDir.y)));

    return randDir * randDistance;
}
