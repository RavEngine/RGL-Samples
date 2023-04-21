#version 460
#extension GL_GOOGLE_include_directive : enable
#include "asteroid_common.glsl"


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec3 outNormal;

layout(std430, binding = 2) buffer asteroidsArgumentBuffer
{
    ArgumentEntry argumentBuffer[];
};

void main() {
    const uint gridSize = 6;

    uint id = argumentBuffer[0].drawId;
    vec3 pos = genAsteroidInitialPosition(id);

    const float scaleFactor = 1;

    float spinSpeed = 1.0;

    mat4 model = mat4(
        vec4(scaleFactor, 0.0, 0.0, 0.0),
        vec4(0.0, scaleFactor, 0.0, 0.0),
        vec4(0.0, 0.0, scaleFactor, 0.0),
        vec4(pos, 1.0)
    );
    
    float angle = ubo.timeSinceStart * rand(vec2(id,id));
    mat4 rotmatx = mat4(
        vec4(1, 0, 0, 0),
        vec4(0, cos(angle), -sin(angle), 0),
        vec4(0, sin(angle), cos(angle), 0),
        vec4(0, 0, 0, 1.0)
    );
    
    mat4 rotmatz = mat4(
       vec4(cos(angle), -sin(angle), 0, 0),
       vec4(sin(angle), cos(angle), 0, 0),
       vec4(0, 0, 0, 0),
       vec4(0, 0, 0, 1)
   );
    
    mat4 finalmat = (model * rotmatx);

    mat3 normalmat = mat3(finalmat);

    vec4 worldpos = finalmat * vec4(inPosition,1);
    
    gl_Position = ubo.viewProj * worldpos;
    outUV = inUV;
    outNormal = normalize(normalmat * inNormal);
}
