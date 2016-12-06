#version 450

in vec3 fragNormal;
in vec3 fragPos;

uniform vec3 eyePos; // !!!

out vec4 color;

void main()
{
    vec3 modelColor = vec3(0.0, 1.0, 0.2);
    vec3 lightPos   = vec3(1.0, 3.0, 0.0);
    vec3 lightColor = vec3(1.0, 1.0, 1.0);

    vec3 fragNormal_norm = normalize(fragNormal);

    // ambient
    float ambient_strength = 0.05;
    vec3 ambient = ambient_strength*modelColor;

    // diffuse
    float diffuse_strength = 0.5;
    vec3 lightDir = normalize(lightPos - fragPos);
    vec3 diffuse = diffuse_strength*max(dot(fragNormal_norm, lightDir), 0.0)*lightColor*modelColor;

    // specular
    float specular_strength = 0.5;
    int specular_pow = 20;
    vec3 viewDir = normalize(eyePos - fragPos);
    vec3 reflDir = reflect(-lightDir, fragNormal_norm);
    vec3 specular = specular_strength*pow(max(dot(viewDir, reflDir), 0.0), specular_pow)*lightColor;

    color = vec4(ambient + diffuse + specular, 1.0);
}
