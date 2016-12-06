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
uniform sampler2D textureShadow; // !!!

uniform int slider;

out vec4 color;


bool inShadow(vec3 lightdir, vec3 normal)
{
    if (int(gl_FragCoord.x) < slider)
      return false;

    vec3 p = (fragPosLight.xyz / fragPosLight.w)*0.5 + 0.5;

    if (p.x < 0.0 || p.x > 1.0 || p.y < 0.0 || p.y > 1.0 || p.z > 1.0)
      return false;

    float d = texture(textureShadow, p.xy).x;

    float bias = max(0.005 * (1.0 - dot(normal, lightdir)), 0.0001);
    return p.z - bias > d;
}


void main()
{
    vec3 texColor = material.useTex ? texture(textureBitmap, fragTexCoor).rgb : vec3(1.0, 1.0, 1.0);
    vec3 lightDir = normalize(light.dir);
    vec3 normal   = normalize(fragNormal);

    // ambient
    vec3 ambient = material.ambient*texColor;

    if (inShadow(lightDir, normal))
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

      color = vec4(ambient + diffuse + specular , 1.0);
    }
}
