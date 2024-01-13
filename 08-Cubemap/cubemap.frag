
layout(binding = 0) uniform sampler cubeSampler;
layout(binding = 1) uniform textureCube tex;


layout(location = 0) in vec3 inUVdir;
layout(location = 0) out vec4 outcolor;

void main(){

	vec3 cubeColor = texture(samplerCube(tex,cubeSampler), inUVdir).xyz;

	outcolor = vec4(cubeColor,1);
}
