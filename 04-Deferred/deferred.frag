#version 450


layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outPosition;

void main() {
    vec3 sampled = vec3(1,1,1);
    
    outColor = vec4(sampled, 1.0);
    
    outNormal = vec4(0,0.5,1,1);
    outPosition = vec4(1,0.5,0,1);
}
