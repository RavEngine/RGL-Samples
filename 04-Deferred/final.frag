#version 450

layout(push_constant) uniform UniformBufferObject{
    ivec2 scdim;
} ubo;

layout(binding = 0) uniform sampler2D finalSampler;

layout(location = 0) out vec4 outColor;

void main() {
    vec2 uv = gl_FragCoord.xy / ubo.scdim;
    vec3 sampled = texture(finalSampler, uv).xyz;
    
    outColor = vec4(sampled, 1.0);
}
