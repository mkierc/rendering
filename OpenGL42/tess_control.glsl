#version 450

layout (vertices = 3) out;

uniform float tessInner;
uniform float tessOuter0;
uniform float tessOuter1;
uniform float tessOuter2;

in vec3 tcColor[];
out vec3 teColor[];

void main()
{
  if (gl_InvocationID == 0)
  {
    gl_TessLevelOuter[0] = tessOuter0;
    gl_TessLevelOuter[1] = tessOuter1;
    gl_TessLevelOuter[2] = tessOuter2;
    gl_TessLevelInner[0] = tessInner;
  }

  gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

  teColor[gl_InvocationID] = tcColor[gl_InvocationID];
}
