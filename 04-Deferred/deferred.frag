
layout(location = 0) in vec2 inUV;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inWorldpos;
layout(location = 3) in vec3 inCubecolor;
layout(location = 4) flat in uint inID;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outPosition;
layout(location = 3) out uint outID;

void main() {
    outColor = vec4(inCubecolor, 1.0);
    
    outNormal = vec4(inNormal,1);
    outPosition = vec4(inWorldpos,1);
    outID = inID;
}
