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
};

in vec3 fragNormal;
in vec4 fragPos;
in vec3 fragColor;

uniform vec3 eyePos;
uniform Light light;
uniform Material material;

out vec4 color;


void main()
{
    vec3 normal = fragNormal;
    if (!gl_FrontFacing)
      normal = -normal;

    // ambient
    vec3 ambient = material.ambient*fragColor;

    // diffuse
    vec3 lightDir = normalize(light.pos - fragPos.xyz);
    vec3 diffuse = max(dot(normal, lightDir), 0.0)*light.color*material.diffuse*fragColor;

    // specular
    float specular_strength = 0.5;
    int specular_pow = 20;
    vec3 viewDir = normalize(eyePos - fragPos.xyz);
    vec3 reflDir = reflect(-lightDir, normal);
    vec3 specular = specular_strength*pow(max(dot(viewDir, reflDir), 0.0), material.shininess)*light.color*material.specular;

    color = vec4(ambient + diffuse + specular, 1.0);
}
