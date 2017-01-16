#version 450

in vec3 position;
in vec2 textureCoor;

uniform mat4 p_matrix;
uniform mat4 v_matrix;
uniform mat4 m_matrix;

out vec2 fragTexCoor;

void main()
{
    mat4 pvm_matrix = p_matrix*v_matrix*m_matrix;

    gl_Position = pvm_matrix*vec4(position, 1.0);

    fragTexCoor = textureCoor - 0.5;
}
