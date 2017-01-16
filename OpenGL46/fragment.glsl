#version 450

out vec4 color;
in vec2 fragTexCoor;
in float fogFactor;

uniform vec3 fogColor;
uniform int slider;

layout (binding = 0) uniform sampler2D textureColor;

void main()
{
  if (int(gl_FragCoord.x) < slider)
    color = texture(textureColor, fragTexCoor);
  else
    color = mix(vec4(fogColor, 1.0), texture(textureColor, fragTexCoor), fogFactor);
}
