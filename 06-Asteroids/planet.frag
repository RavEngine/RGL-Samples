#extension GL_GOOGLE_include_directive : enable
#include "asteroid_common.glsl"

layout(location = 0) in vec2 inUV;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 baseLight = calcBaseLighting(inNormal);
    
    outColor = vec4(baseLight, 1.0);
}
