#version 450

// shadowmap_fragment

in vec4 fragPos;

struct Light
{
    vec3 pos;
    vec3 color;
};

uniform Light light;
uniform float far;

void main()
{
    gl_FragDepth = length(fragPos.xyz - light.pos) / far;
}
