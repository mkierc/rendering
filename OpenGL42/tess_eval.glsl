#version 450

layout (triangles) in;

in vec3 teColor[];
out vec3 fragColor;

void main()
{
  gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position) +
                (gl_TessCoord.y * gl_in[1].gl_Position) +
                (gl_TessCoord.z * gl_in[2].gl_Position);

  fragColor =   (gl_TessCoord.x * teColor[0]) +
                (gl_TessCoord.y * teColor[1]) +
                (gl_TessCoord.z * teColor[2]);
}
