
layout(push_constant) uniform UniformBufferObject{
    mat4 viewProj;
} ubo;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 outUV;

void main(){
    gl_Position = ubo.viewProj * vec4(inPosition, 0, 1);
    outUV = inUV;
}
