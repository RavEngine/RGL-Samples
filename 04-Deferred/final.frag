#version 450

layout(location = 0) out vec4 outColor;

void main() {
    vec3 sampled = vec3(1,1,1);
    
    outColor = vec4(sampled, 1.0);
}