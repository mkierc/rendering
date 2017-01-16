#version 450

struct Light
{
    vec3 pos;
    vec3 color;
};

struct Material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
    bool useTex;
};

in vec3 fragNormal;
in vec4 fragPos;
in vec3 fragTexPos;

uniform vec3 eyePos;
uniform Light light;
uniform Material material;

uniform int slider;

out vec4 color;


vec3 texWood(vec3 p)
{
  const vec3 color1 = vec3(0.94, 0.70, 0.39);
  const vec3 color2 = vec3(0.57, 0.28, 0.00);
//  const vec3 color1 = vec3(149, 74, 42)/255.0;
//  const vec3 color2 = vec3(188, 114, 77)/255.0;

  float f = 8.0*sqrt(p.x*p.x + p.y*p.y);
  float s = f - int(f);
  s = s*s;
  return mix(color1, color2, s);
}

vec3 texMarble(vec3 p)
{
//  const vec3 color2 = vec3(0.0, 0.3, 0.9);
//  const vec3 color1 = vec3(1.0, 1.0, 1.0);

  const vec3 color2 = vec3(145, 212, 220)/255.0;
  const vec3 color1 = vec3(39, 64, 69)/255.0;


  float f = 30.0*p.x;
  float s = sin(f);
  s = s*s;
  return mix(color1, color2, s);
}


void main()
{
    vec3 texColor;
    if (material.useTex)
      if (slider < int(gl_FragCoord.x))
        texColor = texWood(fragTexPos);
      else
        texColor = texMarble(fragTexPos);
    else
      texColor = vec3(1.0, 1.0, 1.0);

    // ambient
    vec3 ambient = material.ambient*texColor;

    // diffuse
    vec3 lightDir = normalize(light.pos - fragPos.xyz);
    vec3 diffuse = max(dot(fragNormal, lightDir), 0.0)*light.color*material.diffuse*texColor;

    // specular
    float specular_strength = 0.5;
    int specular_pow = 20;
    vec3 viewDir = normalize(eyePos - fragPos.xyz);
    vec3 reflDir = reflect(-lightDir, fragNormal);
    vec3 specular = specular_strength*pow(max(dot(viewDir, reflDir), 0.0), material.shininess)*light.color*material.specular;

    color = vec4(ambient + diffuse + specular, 1.0);
}
