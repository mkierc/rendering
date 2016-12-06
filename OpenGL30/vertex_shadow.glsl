#version 450

in vec3 position;

uniform mat4 light_matrix;
uniform mat4 m_matrix;

void main()
{
    gl_Position = light_matrix*m_matrix*vec4(position, 1.0);
}
