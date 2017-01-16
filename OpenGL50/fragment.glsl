#version 450

struct Light
{
    vec3 dir;
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
in vec2 fragTexCoor;
in vec4 fragPosLight; // !!!

uniform vec3 eyePos;
uniform Light light;
uniform Material material;

uniform sampler2D textureBitmap;
uniform sampler2D textureShadow;

uniform int slider;

out vec4 color;



float inShadowPCF(vec3 lightdir, vec3 normal)
{
    vec3 p = (fragPosLight.xyz / fragPosLight.w)*0.5 + 0.5;

    if (p.x < 0.0 || p.x > 1.0 || p.y < 0.0 || p.y > 1.0 || p.z > 1.0)
      return 1.0;

    float bias = max(0.01 * (1.0 - dot(normal, lightdir)), 0.001);
    float shadow = 0.0;
    vec2 texel = 1.0/textureSize(textureShadow, 0);
    for (float i = -1.5; i <= 1.51; i += 1.0)
      for (float j = -1.5; j <= 1.51; j += 1.0)
      {
       float d = texture(textureShadow, p.xy + vec2(i, j)*texel).x;
       shadow += (p.z - bias > d) ? 1.0 : 0.0;
      }

    return 1.0 - shadow/16.0;
}



void main()
{
    vec4 texColor = material.useTex ? texture(textureBitmap, fragTexCoor) : vec4(1.0);

    vec3 lightDir = normalize(light.dir);
    vec3 normal   = normalize(fragNormal);

    // ambient
    vec3 ambient = material.ambient*texColor.rgb;

    // shadow
    float shadow = inShadowPCF(lightDir, normal);

    // diffuse
    vec3 diffuse = max(dot(normal, lightDir), 0.0)*light.color*material.diffuse*texColor.rgb;

    // specular
    float specular_strength = 0.5;
    int specular_pow = 20;
    vec3 viewDir = normalize(eyePos - fragPos.xyz);
    vec3 reflDir = reflect(-lightDir, normal);
    vec3 specular = specular_strength*pow(max(dot(viewDir, reflDir), 0.0), material.shininess)*light.color*material.specular;

    color = vec4(ambient + shadow*(diffuse + specular), texColor.a);
}
