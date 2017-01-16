#version 450

// shading_fragment

in vec2 fragTexCoor;

out vec4 color;
out vec4 colorBright;

layout (location = 0) uniform sampler2D gPosition;
layout (location = 1) uniform sampler2D gNormal;
layout (location = 2) uniform sampler2D gColor;
layout (location = 3) uniform samplerCube textureShadow;

struct Light
{
    vec3 pos;
    vec3 color;
};

uniform Light light;
uniform vec3 eyePos;
uniform float far;


float inShadowPCF(vec3 fPos, vec3 fNormal)
{
    vec3 fragToLight = fPos - light.pos;
    float depth = length(fragToLight);

    if (depth >= far)
        return 1.0;

    float bias = 0.0;//max(0.02 * (1.0 - dot(fNormal, fragToLight)), 0.001);

    float shadow = 0.0;
    float texel = 0.02;
    for (float x = -1.5; x <= 1.51; x += 1.0)
      for (float y = -1.5; y <= 1.51; y += 1.0)
        for (float z = -1.5; z <= 1.51; z += 1.0)
        {
          float closestDepth = texture(textureShadow, fragToLight + vec3(x, y, z)*texel).r * far;
          shadow += depth - bias  > closestDepth ? 1.0 : 0.0;
        }
    return 1.0 - shadow/64.0;
}


float inShadowPCF2(vec3 fPos, vec3 fNormal)
{
    vec3 sample_dir[] = vec3[]
    (
      vec3( 1, 1, 1), vec3( 1, -1, 1), vec3(-1, -1,  1), vec3(-1, 1,  1),
      vec3( 1, 1,-1), vec3( 1, -1,-1), vec3(-1, -1, -1), vec3(-1, 1, -1),
      vec3( 1, 1, 0), vec3( 1, -1, 0), vec3(-1, -1,  0), vec3(-1, 1,  0),
      vec3( 1, 0, 1), vec3(-1,  0, 1), vec3( 1,  0, -1), vec3(-1, 0, -1),
      vec3( 0, 1, 1), vec3( 0, -1, 1), vec3( 0, -1, -1), vec3( 0, 1, -1)
    );


    vec3 fragToLight = fPos - light.pos;
    float depth = length(fragToLight);

    if (depth >= far)
        return 1.0;

    float bias = max(0.02 * (1.0 - dot(fNormal, fragToLight)), 0.001);
    float texel = 0.04;
    float shadow = 0.0;
    for (int i = 0; i < sample_dir.length; i++)
      {
        float closestDepth = texture(textureShadow, fragToLight + sample_dir[i]*texel).r * far;
        shadow +=  depth - bias  > closestDepth ? 1.0 : 0.0;
      }

    return 1.0 - shadow/sample_dir.length;
}


void main()
{
  vec4 p = texture(gPosition, fragTexCoor);
  if (p.w != 0.0)
   discard;

  vec3 fPosition = p.xyz;

  vec3 fNormal   = texture(gNormal, fragTexCoor).xyz;

  p = texture(gColor, fragTexCoor);
  vec3 fColor    = p.rgb;
  float fAmbient = p.a;

  // ambient
  vec3 ambient = fColor * fAmbient;

  // attenuation
  float attenuation = 1.0/length(fPosition - light.pos);

  // diffuse
  vec3 lightDir = normalize(light.pos - fPosition.xyz);
  vec3 diffuse = max(dot(fNormal, lightDir), 0.0)*light.color*fColor*attenuation;

  // specular
  float specular_strength = 0.5;
  int specular_pow = 20;
  vec3 viewDir = normalize(eyePos - fPosition.xyz);
  vec3 reflDir = reflect(-lightDir, fNormal);
  vec3 specular = specular_strength*pow(max(dot(viewDir, reflDir), 0.0), specular_pow)*light.color*attenuation;

  float shadow = inShadowPCF2(fPosition, fNormal);

  color = vec4(ambient + shadow*(diffuse + specular), 1.0);

  if (length(color.rgb) > 2.0)
    colorBright = color;
  else
    colorBright = vec4(0.0);

}
