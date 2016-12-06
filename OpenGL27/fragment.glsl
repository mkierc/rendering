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
uniform Material material;
uniform sampler2D textureColor_front; // !!!
uniform sampler2D textureColor_back; // !!!

out vec4 color;

void main()
{
    vec3 texColor;

    // diffuse
    vec3 lightDir = normalize(light.pos - fragPos.xyz);
    float diffuse_f = dot(fragNormal, lightDir) * (int(gl_FrontFacing)*2-1);
    vec3 diffuse;
    if (diffuse_f > 0)
    {
      texColor = material.useTex ? texture(textureColor_front, fragTexCoor).rgb: vec3(1.0, 1.0, 1.0);
      diffuse = diffuse_f*light.color*material.diffuse*texColor;
    }
    else
    {
      texColor = material.useTex ? texture(textureColor_back, fragTexCoor).rgb: vec3(1.0, 1.0, 1.0);
      diffuse = -diffuse_f*light.color*material.diffuse*texColor;
    }

    // ambient
    vec3 ambient = material.ambient*texColor;

    // specular
    float specular_strength = 0.5;
    int specular_pow = 20;
    vec3 viewDir = normalize(eyePos - fragPos.xyz);
    vec3 reflDir = reflect(-lightDir, fragNormal);
    vec3 specular = specular_strength*pow(max(dot(viewDir, reflDir), 0.0), material.shininess)*light.color*material.specular;

    color = vec4(ambient + diffuse + specular, 1.0);
}
