
layout(push_constant) uniform UniformBufferObject{
    mat4 viewProj;
    float timeSinceStart;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 outUVDir;

void main(){
    
    float scaleFactor = 1;
    
    mat4 model = mat4(
        vec4(scaleFactor, 0.0, 0.0, 0.0),
        vec4(0.0, scaleFactor, 0.0, 0.0),
        vec4(0.0, 0.0, scaleFactor, 0.0),
        vec4(0,0,0, 1.0)
    );
    
    vec4 worldpos = model * vec4(inPosition,1);
    
    gl_Position = ubo.viewProj * worldpos;
    outUVDir = inPosition;
}
