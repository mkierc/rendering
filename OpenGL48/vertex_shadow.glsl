#version 450

layout (location = 0) in vec3 position;
layout (location = 2) in vec2 textureCoor;

uniform mat4 light_matrix;
uniform mat4 m_matrix;

out vec2 fragTexCoor;

void main()
{
    gl_Position = light_matrix*m_matrix*vec4(position, 1.0);
    fragTexCoor = textureCoor;
}
