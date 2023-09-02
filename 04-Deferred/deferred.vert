
layout(push_constant) uniform UniformBufferObject{
    mat4 viewProj;
    vec3 pos;
    float timeSinceStart;
    uint objectID;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outWorldpos;
layout(location = 3) out vec3 outCubeColor;
layout(location = 4) out uint outID;

float rand(vec2 co){
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main() {
    const uint gridSize = 6;

    vec3 pos = ubo.pos;

    const float scaleFactor = 1;

    float spinSpeed = 1.0;

    mat4 model = mat4(
        vec4(scaleFactor, 0.0, 0.0, 0.0),
        vec4(0.0, scaleFactor, 0.0, 0.0),
        vec4(0.0, 0.0, scaleFactor, 0.0),
        vec4(pos, 1.0)
    );

    float animTime = ubo.timeSinceStart * spinSpeed;

    mat4 rotmatx = mat4(
        vec4(1, 0, 0, 0),
        vec4(0, cos(animTime), -sin(animTime), 0),
        vec4(0, sin(animTime), cos(animTime), 0),
        vec4(0, 0, 0, 1.0)
    );

     mat4 rotmatz = mat4(
        vec4(cos(animTime), -sin(animTime), 0, 0),
        vec4(sin(animTime), cos(animTime), 0, 0),
        vec4(0, 0, 0, 0),
        vec4(0, 0, 0, 0)
    );

    mat4 finalmat = model * rotmatx;

    mat3 normalmat = mat3(finalmat);

    
    vec4 worldpos = finalmat * vec4(inPosition,1);
    
    gl_Position = ubo.viewProj * worldpos;
    outUV = inUV;
    outNormal = normalize(normalmat * inNormal);
    outWorldpos = worldpos.xyz;
    outCubeColor = vec3(
        rand(vec2(pos.x,pos.y)),
        rand(vec2(pos.y,pos.z)),
        rand(vec2(pos.z,pos.x))
    );
    outID = ubo.objectID;
}
