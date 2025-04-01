
layout(push_constant) uniform UniformBufferObject{
    mat4 viewProj;
} ubo;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 outUV;
layout(location = 1) out flat uint instance;

layout (binding = 1) readonly buffer bindlessSSBO{
    uint indices[];
};

vec3 positions[] = {
    vec3(-3,0,0),
    vec3(0,0,0),
    vec3(3,0,0)
}; 

void main(){

    instance = indices[gl_InstanceIndex];

    vec3 pos = vec3(inPosition, 0) + positions[gl_InstanceIndex];

    gl_Position = ubo.viewProj * vec4(pos, 1);
    outUV = inUV;
}
