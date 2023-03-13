#version 450

layout(binding = 0) uniform sampler2D colorSampler;
layout(binding = 1) uniform sampler2D normalSampler;
layout(binding = 2) uniform sampler2D positionSampler;

layout(location = 0) out vec4 outColor;

void main() {
    vec2 uv = vec2(0.5,0.5);
    
    vec3 normal = texture(normalSampler,uv).xyz;
    vec3 color = texture(colorSampler,uv).xyz;
    vec3 pos = texture(positionSampler,uv).xyz;
    
    outColor = vec4(color + normal + pos, 1.0);
}
