#version 450

out vec4 color;
in vec2 fragTexCoor;

layout (binding = 0) uniform sampler2D textureColor;

void main()
{
  color = texture(textureColor, fragTexCoor);
}
