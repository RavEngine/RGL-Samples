#extension GL_GOOGLE_include_directive : enable
#include "asteroid_common.glsl"

layout(location = 0) in vec2 inUV;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

// adapted from "dynamic multicolour gradient" by ioblackshaw https://www.shadertoy.com/view/dstSWB
vec3 calcGradient(vec2 uv){
    vec3 colors[] = vec3[](
        vec3(224,213,209) / 255,
        vec3(230,219,227) / 255,
        vec3(167,161,173) / 255,
        vec3(167,161,173) / 255,
        vec3(6,4,17) / 255,
        vec3(169,167,180) / 255,
        vec3(230,219,227) / 255,
        vec3(167,161,173) / 255,
        vec3(6,4,17) / 255,
        vec3(224,213,209) / 255
    );

    vec3 gradient = vec3(0);
    for(int i = 0; i < colors.length(); i++){
        float i_f = float(i);
        float len_f = float(colors.length());

        float _step = i_f/len_f;
        float _step2 = (i_f+1.)/len_f;

        // GRADIENT
        if(i == 0){
            gradient = mix(colors[colors.length()-1], colors[i], smoothstep(_step, _step2, uv.y));
        }else{
            gradient = mix(gradient, colors[i], smoothstep(_step, _step2, uv.y));
        }
    }
    return gradient;
}

void main() {
    vec3 baseLight = calcBaseLighting(inNormal);
    vec3 ringColor = calcGradient(inUV);
    
    outColor = vec4(baseLight * ringColor, 1.0);
}
