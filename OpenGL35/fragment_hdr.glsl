#version 450

in vec2 fragTexCoor;
out vec4 color;

uniform sampler2D textureHDR;
uniform float gamma;
uniform int slider;

const bool grayscale = false;


vec4 JimHejl_RichardBurgessDawson(vec4 color)
{
  vec3 x = max(vec3(0.0), color.rgb - 0.004);

  return vec4((x*(6.2*x+.5))/(x*(6.2*x+1.7)+0.06), 1.0);
}


vec4 Reinhard(vec4 color)
{
  return vec4(color.rgb / (color.rgb + vec3(1.0)), 1.0);
}


vec4 Exposure(vec4 color, float exposure)
{
  return vec4(1.0 - exp2(-color.rgb * exposure), 1.0);
}


void main()
{
  color = texture(textureHDR, fragTexCoor);

  if (int(gl_FragCoord.x) >= slider)
    color = Reinhard(color);

  if (grayscale)
  {
    float gs = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
    color = vec4(gs, gs, gs, 1.0);
  }


  // Gamma correction
  color = pow(color, vec4(1.0 / gamma));
}
