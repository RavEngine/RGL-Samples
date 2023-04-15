#version 460
#extension GL_GOOGLE_include_directive : enable
#include "asteroid_common.glsl"


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec3 outNormal;


void main() {
    const uint gridSize = 6;

    vec3 pos = genAsteroidInitialPosition(gl_DrawID);

    const float scaleFactor = 1;

    float spinSpeed = 1.0;

    mat4 model = mat4(
        vec4(scaleFactor, 0.0, 0.0, 0.0),
        vec4(0.0, scaleFactor, 0.0, 0.0),
        vec4(0.0, 0.0, scaleFactor, 0.0),
        vec4(pos, 1.0)
    );
    
    mat4 finalmat = model;

    mat3 normalmat = transpose(mat3(finalmat));

    vec4 worldpos = finalmat * vec4(inPosition,1);
    
    gl_Position = ubo.viewProj * worldpos;
    outUV = inUV;
    outNormal = normalize(normalmat * inNormal);
}
