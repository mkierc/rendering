#version 450

layout (quads) in;


vec4 quadraticBezier(vec4 A, vec4 B, vec4 C, float t)
{
  vec4 D = mix(A, B, t);
  vec4 E = mix(B, C, t);
  return mix(D, E, t);
}


vec4 cubicBezier(vec4 A, vec4 B, vec4 C, vec4 D, float t)
{
  vec4 E = mix(A, B, t);
  vec4 F = mix(B, C, t);
  vec4 G = mix(C, D, t);
  return quadraticBezier(E, F, G, t);
}


vec4 evaluatePatch(vec2 where)
{
  vec4 P[4];
  int i;
  for (i = 0; i < 4; i++)
  {
    P[i] = cubicBezier(gl_in[i + 0].gl_Position,
                       gl_in[i + 4].gl_Position,
                       gl_in[i + 8].gl_Position,
                       gl_in[i + 12].gl_Position,
                       where.y);
  }
  return cubicBezier(P[0], P[1], P[2], P[3], where.x);
}


void main(void)
{
  gl_Position = evaluatePatch(gl_TessCoord.xy);
}

