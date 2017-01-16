#version 450

layout (quads) in;

in vec3 teColor[];
out vec3 fragColor;

void main()
{
  vec4 s01 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.y);
  vec4 s32 = mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.y);

  gl_Position = mix(s01, s32, gl_TessCoord.x);

  vec3 c01 = mix(teColor[0], teColor[1], gl_TessCoord.y);
  vec3 c32 = mix(teColor[3], teColor[2], gl_TessCoord.y);

  fragColor = mix(c01, c32, gl_TessCoord.x);
}
