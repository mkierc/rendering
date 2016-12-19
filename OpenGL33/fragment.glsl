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
uniform samplerCube textureShadow; // !!!

uniform int slider;

out vec4 color;



bool inShadow()
{
    if (int(gl_FragCoord.x) < slider)
        return false;

    vec3 fragToLight = fragPos.xyz - light.pos;
    float depth = length(fragToLight);

    if (depth >= far)
        return false;

    float closestDepth = texture(textureShadow, fragToLight).r * far;

    float bias = max(0.02 * (1.0 - dot(fragNormal, fragToLight)), 0.0001);
    return depth - bias  > closestDepth;
}

void main()
{
    vec3 texColor = material.useTex ? texture(textureColor, fragTexCoor).rgb : vec3(1.0, 1.0, 1.0);
    vec3 lightDir = normalize(light.pos - fragPos.xyz);
    vec3 normal   = normalize(fragNormal);

    // ambient
    vec3 ambient = material.ambient*texColor;

    if (inShadow())
      color = vec4(ambient, 1.0);
    else
    {
      // diffuse
      vec3 diffuse = max(dot(normal, lightDir), 0.0)*light.color*material.diffuse*texColor;

      // specular
      float specular_strength = 0.5;
      int specular_pow = 20;
      vec3 viewDir = normalize(eyePos - fragPos.xyz);
      vec3 reflDir = reflect(-lightDir, normal);
      vec3 specular = specular_strength*pow(max(dot(viewDir, reflDir), 0.0), material.shininess)*light.color*material.specular;

      color = vec4(ambient + diffuse + specular, 1.0);
    }
}
