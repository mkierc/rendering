#version 450

in vec3 position;
in vec3 normal;
uniform mat4 pvm_matrix;
uniform mat3 norm_matrix;

out vec3 fragNormal;

void main()
{
  gl_Position = pvm_matrix*vec4(position, 1.0);
  fragNormal = normalize(norm_matrix*normal); // !!!
}
