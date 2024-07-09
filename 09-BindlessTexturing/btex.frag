#extension GL_EXT_nonuniform_qualifier : enable

layout(binding = 0) uniform sampler g_sampler;
layout(set = 1, binding = 0) uniform texture2D textures[];

layout(location = 0) in vec2 uv;
layout(location = 1) in flat uint instance;
layout(location = 0) out vec4 outcolor;
void main(){
    
    outcolor = vec4(texture(sampler2D(textures[instance], g_sampler),uv).rgb,1);
}
