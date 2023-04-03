
float rand(vec2 co){
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec3 genAsteroidInitialPosition(uint pos){
    return vec3(
                rand(vec2(pos,-pos)),
                rand(vec2(-pos,pos)),
                rand(vec2(-pos,-pos))
                );
}
