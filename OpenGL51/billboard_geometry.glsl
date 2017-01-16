#version 450

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform mat4 p_matrix;
uniform mat4 v_matrix;
uniform mat4 m_matrix;
uniform float v_ratio;
uniform float d;

out vec2 texCoor;

/*

2 (0,0)    3 (1,0)
     +------+
     |\     |
     | \    |
     |  \   |
     |   \  |
     |    \ |
     |     \|
     +------+
0 (0,1)    1 (1,1)

*/


void main()
{
  vec4 v1 = v_matrix*m_matrix*gl_in[0].gl_Position;
  vec4 v2 = v_matrix*m_matrix*(gl_in[0].gl_Position + vec4(d, 0, 0, 0));

  float sy = length(v2 - v1);
  float sx = sy*v_ratio;

  mat4 pvm_matrix = p_matrix*v_matrix*m_matrix;

  texCoor = vec2(0, 1);
  gl_Position = pvm_matrix*gl_in[0].gl_Position + vec4(-sx, -sy, 0.0, 0.0);
  EmitVertex();

  texCoor = vec2(1, 1);
  gl_Position = pvm_matrix*gl_in[0].gl_Position + vec4(sx,  -sy, 0.0, 0.0);
  EmitVertex();

  texCoor = vec2(0, 0);
  gl_Position = pvm_matrix*gl_in[0].gl_Position + vec4(-sx, sy, 0.0, 0.0);
  EmitVertex();

  texCoor = vec2(1, 0);
  gl_Position = pvm_matrix*gl_in[0].gl_Position + vec4(sx, sy, 0.0, 0.0);
  EmitVertex();

  EndPrimitive();
}
