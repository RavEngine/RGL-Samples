#version 450

uniform usampler2D texSampler;

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 sampled = texture(texSampler,inUV).rgb / 255.0;
    
    outColor = vec4(sampled, 1.0);
}
