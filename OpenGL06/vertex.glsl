#version 450

in vec3 position;
in vec3 color;

out vec3 fragcolor;

void main()
{
    gl_Position = vec4(position, 1.0);
    fragcolor = color;
}
