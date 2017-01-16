#version 450

in vec3 position;
in vec2 texCoor;

uniform mat4 pvm_matrix;

out vec2 fragTexCoor;

void main()
{
    gl_Position = pvm_matrix*vec4(position, 1.0);
    fragTexCoor = texCoor;
}
