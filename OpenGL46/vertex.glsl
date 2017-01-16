#version 450

in vec3 position;

uniform int  grid_size;

out vec2 tcsCoor;

void main()
{
  int grid_x = gl_InstanceID % grid_size;
  int grid_z = gl_InstanceID / grid_size;
  float shift = (grid_size - 1)*0.5;
  vec3 p = position + vec3(grid_x - shift, 0, grid_z - shift);

  gl_Position = vec4(p, 1.0);

  tcsCoor = (position.xz + vec2(0.5) + vec2(grid_x, grid_z))*(1.0/grid_size);
}
