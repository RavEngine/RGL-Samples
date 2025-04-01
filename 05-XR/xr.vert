
layout(push_constant) uniform UniformBufferObject{
    mat4 viewProj;
    float timeSinceStart;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 0) out vec3 vertColor;

vec3 vertColors[] = {
    vec3(1,0,0),
    vec3(0,1,0),
    vec3(0,0,1)
};

void main() {
    const uint gridSize = 6;

    vec3 pos = vec3(0, 0, -10);

    const float scaleFactor = 1;

    float spinSpeed = 1;

    mat4 model = mat4(
        vec4(scaleFactor, 0.0, 0.0, 0.0),
        vec4(0.0, scaleFactor, 0.0, 0.0),
        vec4(0.0, 0.0, scaleFactor, 0.0),
        vec4(pos, 1.0)
    );

    float animTime = ubo.timeSinceStart * spinSpeed;

    mat4 rotmatx = mat4(
        vec4(1, 0, 0, 0),
        vec4(0, cos(animTime), -sin(animTime), 0),
        vec4(0, sin(animTime), cos(animTime), 0),
        vec4(0, 0, 0, 1.0)
    );

     mat4 rotmatz = mat4(
        vec4(cos(animTime), -sin(animTime), 0, 0),
        vec4(sin(animTime), cos(animTime), 0, 0),
        vec4(0, 0, 0, 0),
        vec4(0, 0, 0, 0)
    );
    
    vec4 worldpos = (model *  rotmatx) * vec4(inPosition,1);
    
    gl_Position = ubo.viewProj * worldpos;
    vertColor = vertColors[gl_VertexIndex % 3];
}
