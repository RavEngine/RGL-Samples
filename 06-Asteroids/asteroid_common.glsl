
float rand(vec2 co){
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec3 genAsteroidInitialPosition(uint pos){
    const float maxScale = 100;
    vec3 randpos = vec3(
                rand(vec2(pos,-int(pos))),
                rand(vec2(-int(pos),pos)),
                rand(vec2(-int(pos),-int(pos)))
                );

    return (randpos * rand(vec2(randpos.x,randpos.y))) * maxScale;
}
