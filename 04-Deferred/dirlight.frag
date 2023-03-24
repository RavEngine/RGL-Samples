#version 450

layout(push_constant) uniform UniformBufferObject{
    ivec2 scdim;
} ubo;

layout(binding = 0) uniform sampler2D colorSampler;
layout(binding = 1) uniform sampler2D normalSampler;
layout(binding = 2) uniform sampler2D positionSampler;

layout(location = 0) out vec4 outColor;

void main() {
    vec2 uv = gl_FragCoord.xy / ubo.scdim;
    
    vec3 normal = texture(normalSampler,uv).xyz;
    vec3 color = texture(colorSampler,uv).xyz;
    vec3 pos = texture(positionSampler,uv).xyz;

    vec3 toLight = -vec3(0,-1,0);
    float ndotl = max(dot(normal,toLight), 0);

    vec3 diffuse = color * ndotl;
    float intensity = 1;
    vec3 lightColor = vec3(1,1,1);

    vec3 ambientlight = vec3(0.1,0.1,0.1);
    
    outColor = vec4((intensity * lightColor * diffuse) + ambientlight, 1.0);
}
