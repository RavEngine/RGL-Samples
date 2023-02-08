#version 450

layout(push_constant) uniform UniformBufferObject{
    mat4 viewProj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec2 outUV;

void main() {
    mat4 model = mat4(1);
    
    vec4 worldpos = model * vec4(inPosition,1);
    
    gl_Position = ubo.viewProj * worldpos;
    outUV = inUV;
}
