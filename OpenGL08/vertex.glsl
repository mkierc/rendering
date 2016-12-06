#version 450

in vec3 position;
in vec3 normal;

out vec4 fragNormal;

void main()
{
    gl_Position = vec4(position, 1.0);
    fragNormal = vec4(normal, 1.0);
}
