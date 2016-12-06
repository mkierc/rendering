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

//in vec4 fragPos;
in vec2 fragTexCoor;
in vec3 fragTanEyePos;
in vec3 fragTanLightPos;
in vec3 fragTanFragPos;
in vec3 fragTanNormal;

uniform vec3 eyePos;
uniform Light light;
uniform Material material;
uniform sampler2D textureColor;
uniform sampler2D textureNormal;

uniform int slider;

out vec4 color;

void main()
{
    vec3 texColor = texture(textureColor, fragTexCoor).rgb * material.useTex + vec3(1.0 - material.useTex, 1.0 - material.useTex, 1.0 - material.useTex);
    vec3 texNormal = normalize(texture(textureNormal, fragTexCoor).rgb * 2.0 - 1.0);

    if (int(gl_FragCoord.x) < slider)
      texNormal = vec3(0.0, 0.0, 1.0);


    // ambient
    vec3 ambient = material.ambient*texColor;

    // diffuse
    vec3 lightDir = normalize(fragTanLightPos - fragTanFragPos);
    vec3 diffuse = max(dot(texNormal, lightDir), 0.0)*light.color*material.diffuse*texColor;

    // specular
    vec3 viewDir = normalize(fragTanEyePos - fragTanFragPos);
    vec3 reflDir = reflect(-lightDir, texNormal);
    vec3 specular = pow(max(dot(viewDir, reflDir), 0.0), material.shininess + 1)*light.color*material.specular;

    float v = max(dot(fragTanNormal, lightDir), 0.0);
    if (v < 0.1)
    {
      diffuse *= v*10;
      specular *= v*10;
    }

    color = vec4(ambient + diffuse + specular, 1.0);
}
