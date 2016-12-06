#version 450

in vec3 position;

uniform mat4 p_matrix;
uniform mat4 v_matrix;

out vec3 fragTexCoor;

void main()
{
    mat4 pv_matrix = inverse(p_matrix*v_matrix);

    gl_Position = vec4(position, 1.0);

    fragTexCoor = (pv_matrix*vec4(position, 1.0)).xyz;
}
