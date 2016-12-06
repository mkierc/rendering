#version 450

layout (triangles) in;
layout (line_strip, max_vertices=6) out;

in vec3 geoNormal[];
in vec4 geoPos[];

out vec3 fragNormal;
out vec4 fragPos;
out vec4 fragColor;
out flat float useColor;

uniform mat4 p_matrix;
uniform mat4 v_matrix;
uniform mat4 m_matrix;

uniform int slider;


void main()
{
  // trojkat

  useColor = 0;
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

  fragNormal = geoNormal[0];
  fragPos = geoPos[0];
  gl_Position = gl_in[0].gl_Position;
  EmitVertex();

  EndPrimitive();


  // normalna


  useColor = 1;
  mat4 pvm_matrix = p_matrix*v_matrix*m_matrix;

  fragNormal = (geoNormal[0] + geoNormal[1] + geoNormal[2])/3.0;
  fragColor = vec4(abs(fragNormal), 1.0);
  fragPos = (geoPos[0] + geoPos[1] + geoPos[2])/3.0;
  gl_Position = (gl_in[0].gl_Position + gl_in[1].gl_Position + gl_in[2].gl_Position)/3.0;
  EmitVertex();

  fragPos += vec4(fragNormal, 0.0);
  gl_Position += pvm_matrix*vec4(fragNormal, 0.0)*0.001*slider;
  EmitVertex();

  EndPrimitive();
}
