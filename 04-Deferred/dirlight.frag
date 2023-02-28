#version 450

uniform sampler2D colorSampler;
uniform sampler2D normalSampler;
uniform sampler2D positionSampler;

layout(location = 0) out vec4 outColor;

void main() {
    vec2 uv = vec2(0.5,0.5);
    
    vec3 color = texture(colorSampler,uv).xyz;
    vec3 normal = texture(normalSampler,uv).xyz;
    vec3 pos = texture(positionSampler,uv).xyz;
    
    outColor = vec4(color + normal + pos, 1.0);
}
