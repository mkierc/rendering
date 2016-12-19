#version 450

in vec3 position;

out vec2 fragTexCoor;

void main()
{
  gl_Position = vec4(position, 1.0);
  fragTexCoor = position.xy*0.5 + vec2(0.5);
}
