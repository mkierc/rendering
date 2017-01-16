#version 450

struct Light
{
    vec3 pos;
    vec3 color;
};

uniform Light light;

out vec4 color;

void main()
{
    color = vec4(light.color, 1.0);
}
