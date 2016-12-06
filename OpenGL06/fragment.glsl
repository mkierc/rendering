#version 450

in vec3 fragcolor;

out vec4 color;

void main()
{
    color = vec4(fragcolor, 1.0);
}
