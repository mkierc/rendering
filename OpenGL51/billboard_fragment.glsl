#version 450

in vec2 texCoor;

uniform sampler2D tex;
uniform int slider;

out vec4 color;

void main()
{
  if (slider > int(gl_FragCoord.x))
  {
    if (length(texCoor - vec2(0.5, 0.5)) < 0.01)
      color = vec4(1.0);
    else
      discard;
    return;
  }

  color = texture(tex, texCoor);
}
