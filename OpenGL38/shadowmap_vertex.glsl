#version 450

// shadowmap_vertex

in vec3 position;

uniform mat4 m_matrix;

void main()
{
    gl_Position = m_matrix*vec4(position, 1.0);
}
