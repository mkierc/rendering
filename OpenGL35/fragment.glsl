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
in vec2 fragTexCoor;

uniform vec3 eyePos;
uniform Light light;
uniform float far;
uniform Material material;

uniform sampler2D textureColor;
uniform samplerCube textureShadow;

out vec4 color;


float inShadowPCF()
{
    vec3 fragToLight = fragPos.xyz - light.pos;
    float depth = length(fragToLight);

    if (depth >= far)
        return 1.0;

    float bias = max(0.02 * (1.0 - dot(fragNormal, fragToLight)), 0.001);

    float shadow = 0.0;
    float texel = 0.02;
    for (float x = -1.5; x <= 1.51; x += 1.0)
      for (float y = -1.5; y <= 1.51; y += 1.0)
        for (float z = -1.5; z <= 1.51; z += 1.0)
        {
          float closestDepth = texture(textureShadow, fragToLight + vec3(x, y, z)*texel).r * far;
          shadow +=  depth - bias  > closestDepth ? 1.0 : 0.0;
        }
    return 1.0 - shadow/64.0;
}




float inShadowPCF2()
{
    vec3 sample_dir[] = vec3[]
    (
      vec3( 1, 1, 1), vec3( 1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
      vec3( 1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
      vec3( 1, 1, 0), vec3( 1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
      vec3( 1, 0, 1), vec3(-1, 0, 1), vec3( 1, 0, -1), vec3(-1, 0, -1),
      vec3( 0, 1, 1), vec3( 0, -1, 1), vec3( 0, -1, -1), vec3( 0, 1, -1)
    );


    vec3 fragToLight = fragPos.xyz - light.pos;
    float depth = length(fragToLight);

    if (depth >= far)
        return 1.0;

    float bias = max(0.02 * (1.0 - dot(fragNormal, fragToLight)), 0.001);
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
    vec3 texColor = material.useTex ? texture(textureColor, fragTexCoor).rgb : vec3(1.0, 1.0, 1.0);
    vec3 lightDir = normalize(light.pos - fragPos.xyz);
    vec3 normal   = normalize(fragNormal);

    float attenuation = 1.0/length(fragPos.xyz - light.pos); // !!!

    // ambient
    vec3 ambient = material.ambient*texColor;

    // diffuse
    vec3 diffuse = max(dot(normal, lightDir), 0.0)*light.color*material.diffuse*texColor*attenuation;

    // specular
    float specular_strength = 0.5;
    int specular_pow = 20;
    vec3 viewDir = normalize(eyePos - fragPos.xyz);
    vec3 reflDir = reflect(-lightDir, normal);
    vec3 specular = specular_strength*pow(max(dot(viewDir, reflDir), 0.0), material.shininess)*light.color*material.specular*attenuation;

    float shadow = inShadowPCF();
    color = vec4(ambient + shadow*(diffuse + specular), 1.0);
}
