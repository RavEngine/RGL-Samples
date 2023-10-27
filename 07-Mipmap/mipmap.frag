
layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outcolor;
void main(){
    outcolor = vec4(uv,1,1);
}
