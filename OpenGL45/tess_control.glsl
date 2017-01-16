#version 450

layout (vertices = 4) out;

in vec2 tcsCoor[];

uniform mat4 pvm_matrix;

out vec2 tesCoor[];


const float density = 32;
void main()
{
  if (gl_InvocationID == 0)
  {
    vec4 p[4];
    for (int i = 0; i < 4; i++)
    {
      p[i] = pvm_matrix*gl_in[i].gl_Position;
      p[i] /= p[i].w;
    }

    float l[4];
    l[0] = length(p[0] - p[1])*density + 1.0;
    l[1] = length(p[0] - p[3])*density + 1.0;
    l[2] = length(p[2] - p[3])*density + 1.0;
    l[3] = length(p[1] - p[2])*density + 1.0;

    gl_TessLevelOuter[0] = l[0];
    gl_TessLevelOuter[1] = l[1];
    gl_TessLevelOuter[2] = l[2];
    gl_TessLevelOuter[3] = l[3];

    gl_TessLevelInner[0] = min(l[1], l[3]);
    gl_TessLevelInner[1] = min(l[0], l[2]);
  }

  gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
  tesCoor[gl_InvocationID] = tcsCoor[gl_InvocationID];
}
