#version 450

layout(binding = 0) uniform UniformBufferObject{
    float time;
} ubo;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {

    float anim = ubo.time / 100;
    mat2 rotmat = {
		vec2(cos(anim),-sin(anim)),
		vec2(sin(anim), cos(anim))
	};

    gl_Position = vec4(rotmat * inPosition, 0.5, 1.0);
    fragColor = inColor;
}