#version 450

in vec3 position;
in vec3 normal;

uniform mat4 pvm_matrix;

out vec4 fragNormal;

void main()
{
    gl_Position = pvm_matrix*vec4(position, 1.0);
    fragNormal = vec4(normal, 1.0);
}
