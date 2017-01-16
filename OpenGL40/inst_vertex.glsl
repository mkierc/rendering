#version 450

in vec3 position;
in vec3 normal;

uniform mat4 p_matrix;
uniform mat4 v_matrix;
uniform mat4 m_matrix;

uniform float size;
uniform int   side;

out vec3 fragNormal;
out vec4 fragPos;
out vec3 fragColor;



float seed = 1.0;
float rand(float minv, float maxv)
{
  float r = 0.5 + 0.5 * fract(sin(dot(vec2(gl_InstanceID+seed, gl_InstanceID*seed), vec2(12.9898, 78.233)))* 43758.5453);
  seed = r;

  return r*(maxv - minv) + minv;
}


void main()
{
  vec3 offset = vec3(gl_InstanceID % side, 0.0, gl_InstanceID / side);
  offset = offset/float(side) - vec3(0.5, 0.0, 0.5);


  mat4 m_matrix_inst = m_matrix;
  mat4 m;

  // przesuniecie na plaszczyznie XZ:
  offset += vec3(rand(-1.0/side, 1.0/side), 0.0, rand(-1.0/side, 1.0/side));
  m[0] = vec4(1.0, 0.0, 0.0, 0.0);
  m[1] = vec4(0.0, 1.0, 0.0, 0.0);
  m[2] = vec4(0.0, 0.0, 1.0, 0.0);
  m[3] = vec4(offset.x*size*0.5, 0.2*sin(offset.x*10) - 0.2*sin(offset.z*13), offset.z*size*0.5, 1.0);
  m_matrix_inst = m_matrix_inst*m;

  // randomowa rotacja wokol Y:
  float rotY = rand(0.0, 360.0);
  float s = sin(rotY);
  float c = cos(rotY);
  m[0] = vec4(c, 0.0, s, 0.0);
  m[1] = vec4(0.0, 1.0, 0.0, 0.0);
  m[2] = vec4(-s, 0.0, c, 0.0);
  m[3] = vec4(0.0, 0.0, 0.0, 1.0);
  m_matrix_inst = m_matrix_inst*m;

  // randomowa dlugosc
  m[0] = vec4(1.0, 0.0, 0.0, 0.0);
  m[1] = vec4(0.0, rand(0.75, 1.5), 0.0, 0.0);
  m[2] = vec4(0.0, 0.0, 1.0, 0.0);
  m[3] = vec4(0.0, 0.0, 0.0, 1.0);
  m_matrix_inst = m_matrix_inst*m;

  mat4 pvm_matrix = p_matrix*v_matrix*m_matrix_inst;

  mat3 n_matrix = mat3(transpose(inverse(m_matrix_inst)));

  gl_Position = pvm_matrix*vec4(position, 1.0);
  fragPos = m_matrix_inst*vec4(position, 1.0);
  fragNormal = normalize(n_matrix*normal);
  fragColor = vec3(0.0, rand(0.8, 1.0), rand(0.0, 0.3));
}
