#version 450

// shading_fragment

in vec2 fragTexCoor;

out vec4 color;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColor;
uniform sampler2D gId;
uniform int width;
uniform int height;
uniform int selected_id;

struct Light
{
    vec3 pos;
    vec3 color;
};

uniform Light light;
uniform vec3 eyePos;

uniform int slider;


void main()
{
  vec4 p = texture(gPosition, fragTexCoor);
  if (p.w != 0.0)
    discard;

  vec3 fPosition = p.xyz;
  vec3 fNormal   = texture(gNormal,   fragTexCoor).xyz;

  vec4 c = texture(gColor,    fragTexCoor);
  vec3 fColor    = c.rgb;
  float fAmbient = c.a;

  // ambient
  vec3 ambient = fColor * fAmbient;

  // diffuse
  vec3 lightDir = normalize(light.pos - fPosition.xyz);
  vec3 diffuse = max(dot(fNormal, lightDir), 0.0)*light.color*fColor;

  // specular
  float specular_strength = 0.5;
  int specular_pow = 20;
  vec3 viewDir = normalize(eyePos - fPosition.xyz);
  vec3 reflDir = reflect(-lightDir, fNormal);
  vec3 specular = specular_strength*pow(max(dot(viewDir, reflDir), 0.0), specular_pow)*light.color;

  color = vec4(ambient + diffuse + specular, 1.0);

  float id = texture(gId, fragTexCoor).r;

  if (slider > int(gl_FragCoord.x))
    color = vec4(abs(sin(id)), abs(sin(id*7)) ,abs(sin(id*3)), 1.0);

  if (selected_id == id)
  {
    const int thickness = 1;
    vec2 texelSize = vec2(1.0/width, 1.0/height);
    for (int i = -thickness; i <= thickness; i++)
      for (int j = -thickness; j <= thickness; j++)
        if (i != 0 || j != 0)
        {
          float id2 = texture(gId, fragTexCoor + texelSize*vec2(i, j)).r;
          if (id != id2)
          {
            color = vec4(1.0, 0.0, 0.0, 1.0);
            return;
          }
        }
  }

}
