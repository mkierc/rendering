#version 450

in vec2 texCoor;

uniform sampler2D tex;
uniform float alpha;

out vec4 color;

void main()
{
  color = texture(tex, texCoor);
  color.a *= alpha;
}
