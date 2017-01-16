#version 450

in vec2 fragTexCoor;
uniform sampler2D textureBitmap;


void main()
{
  vec4 texColor = texture(textureBitmap, fragTexCoor);

  if (texColor.a < 0.5)
    discard;
}

