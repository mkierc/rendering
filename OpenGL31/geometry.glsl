#version 450

layout (triangles) in;
layout (triangle_strip, max_vertices=3) out;

in vec3 geoNormal[];
in vec4 geoPos[];

out vec3 fragNormal;
out vec4 fragPos;


void main()
{
  fragNormal = geoNormal[0];
  fragPos = geoPos[0];
  gl_Position = gl_in[0].gl_Position;
  EmitVertex();

  fragNormal = geoNormal[1];
  fragPos = geoPos[1];
  gl_Position = gl_in[1].gl_Position;
  EmitVertex();

  fragNormal = geoNormal[2];
  fragPos = geoPos[2];
  gl_Position = gl_in[2].gl_Position;
  EmitVertex();

  EndPrimitive();
}
