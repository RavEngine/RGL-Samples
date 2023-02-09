#version 450

layout(push_constant) uniform UniformBufferObject{
    mat4 viewProj;
    float timeSinceStart;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec2 outUV;

void main() {
    const uint gridSize = 5;

    vec3 pos = vec3(
        gl_InstanceID % gridSize - gridSize/2.0f,   // column
        gl_InstanceID / gridSize - gridSize/2.0f,    // row
        0
    );

    const float scaleFactor = 1;



    mat4 model = mat4(
        vec4(scaleFactor, 0.0, 0.0, 0.0),
        vec4(0.0, scaleFactor, 0.0, 0.0),
        vec4(0.0, 0.0, scaleFactor, 0.0),
        vec4(pos, 1.0)
    );

    mat4 rotmatx = mat4(
        vec4(1, 0, 0, 0),
        vec4(0, cos(ubo.timeSinceStart), -sin(ubo.timeSinceStart), 0),
        vec4(0, sin(ubo.timeSinceStart), cos(ubo.timeSinceStart), 0),
        vec4(0, 0, 0, 1.0)
    );

     mat4 rotmatz = mat4(
        vec4(cos(ubo.timeSinceStart), -sin(ubo.timeSinceStart), 0, 0),
        vec4(sin(ubo.timeSinceStart), cos(ubo.timeSinceStart), 0, 0),
        vec4(0, 0, 0, 0),
        vec4(0, 0, 0, 0)
    );
    
    vec4 worldpos = (model *  rotmatx) * vec4(inPosition,1);
    
    gl_Position = ubo.viewProj * worldpos;
    outUV = inUV;
}
