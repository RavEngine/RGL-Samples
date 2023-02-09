#version 450

uniform sampler2D texSampler;

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 sampled = texture(texSampler,inUV).rgb;
    
    outColor = vec4(sampled, 1.0);
}
