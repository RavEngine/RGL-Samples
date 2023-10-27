
layout(binding = 0) uniform sampler g_sampler;
layout(binding = 1) uniform texture2D tex;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outcolor;
void main(){
    vec3 sampled = texture(sampler2D(tex,g_sampler),uv).rgb;
    
    outcolor = vec4(sampled,1);
}
