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

uniform sampler2D textureColor;
uniform sampler2D textureSpecular;

uniform int slider;

out vec4 color;

void main()
{
    vec3 texColor = material.useTex ? texture(textureColor, fragTexCoor).rgb : vec3(1.0, 1.0, 1.0);
    vec3 texSpec = texture(textureSpecular, fragTexCoor).rgb;

    if (int(gl_FragCoord.x) < slider)
      texSpec = material.specular;

    // ambient
    vec3 ambient = material.ambient*texColor;

    // diffuse
    vec3 lightDir = normalize(light.pos - fragPos.xyz);
    vec3 diffuse = max(dot(fragNormal, lightDir), 0.0)*light.color*material.diffuse*texColor;

    // specular
    float shininess = texSpec.g;
    float specular_strength = texSpec.r;
    vec3 viewDir = normalize(eyePos - fragPos.xyz);
    vec3 reflDir = reflect(-lightDir, fragNormal);
    vec3 specular = specular_strength*pow(max(dot(viewDir, reflDir), 0.0), material.shininess*shininess + 1)*light.color*material.specular;

    color = vec4(ambient + diffuse + specular, 1.0);
}
