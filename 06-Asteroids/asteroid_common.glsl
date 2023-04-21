
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

float rand(vec2 co){
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
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

vec3 genAsteroidInitialPosition(uint pos){
    const float maxScale = 100;
    vec3 randDir = vec3(
                rand(vec2(pos,-int(pos))),
                rand(vec2(-int(pos),pos)),
                rand(vec2(-int(pos),-int(pos)))
                );
    randDir = normalize(randDir);

    return (randDir * rand(vec2(randDir.x,randDir.y))) * maxScale;
}
