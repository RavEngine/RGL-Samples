#version 450

layout(binding = 0) uniform sampler2D finalSampler;

layout(location = 0) out vec4 outColor;

void main() {
    vec2 uv = vec2(0.5,0.5);
    vec3 sampled = texture(finalSampler, uv).xyz;
    
    outColor = vec4(sampled, 1.0);
}
