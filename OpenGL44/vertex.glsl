#version 450

in vec3 position;

uniform mat4 pvm_matrix;

out vec3 tcColor;

void main()
{
  gl_Position = pvm_matrix*vec4(position, 1.0);
}
