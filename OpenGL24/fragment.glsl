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
    float useTex;
};

in vec4 fragPos;
in vec2 fragTexCoor;
in mat3 fragTBN;


uniform vec3 eyePos;
uniform Light light;
uniform Material material;
uniform sampler2D textureColor;
uniform sampler2D textureNormal;

out vec4 color;

void main()
{
    vec3 texColor = texture(textureColor, fragTexCoor).rgb * material.useTex + vec3(1.0 - material.useTex, 1.0 - material.useTex, 1.0 - material.useTex);
    vec3 texNormal = texture(textureNormal, fragTexCoor).rgb * 2.0 - 1.0;

    texNormal = normalize(fragTBN*texNormal);

    // ambient
    vec3 ambient = material.ambient*texColor;

    // diffuse
    vec3 lightDir = normalize(light.pos - fragPos.xyz);
    vec3 diffuse = max(dot(texNormal, lightDir), 0.0)*light.color*material.diffuse*texColor;

    // specular
    vec3 viewDir = normalize(eyePos - fragPos.xyz);
    vec3 reflDir = reflect(-lightDir, texNormal);
    vec3 specular = pow(max(dot(viewDir, reflDir), 0.0), material.shininess + 1)*light.color*material.specular;

    float v = max(dot(fragTBN[2], lightDir), 0.0);
    if (v < 0.1)
    {
      diffuse *= v*10;
      specular *= v*10;
    }

    color = vec4(ambient + diffuse + specular, 1.0);
}
