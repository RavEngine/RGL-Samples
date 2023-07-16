#version 450

layout(push_constant) uniform UniformBufferObject{
    ivec2 scdim;
} ubo;

layout(binding = 0) uniform sampler g_sampler;
layout(binding = 1) uniform texture2D t_final;

layout(location = 0) out vec4 outColor;

void main() {
    vec2 uv = gl_FragCoord.xy / ubo.scdim;
    vec3 sampled = texture(sampler2D(t_final,g_sampler), uv).xyz;
    
    outColor = vec4(sampled, 1.0);
}
