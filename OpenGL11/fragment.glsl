#version 450

in vec4 fragNormal;

out vec4 color;

void main()
{
    color = abs(fragNormal);
}
