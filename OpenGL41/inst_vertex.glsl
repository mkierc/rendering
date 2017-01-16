#version 450

in vec3 position;
in vec3 normal;
in vec2 textureCoor;
in vec3 instancePosition; // !!!
in vec3 instanceColor;// !!!

uniform mat4 p_matrix;
uniform mat4 v_matrix;
uniform mat4 m_matrix;

out vec3 fragNormal;
out vec4 fragPos;
out vec2 fragTexCoor;
out vec3 fragColor; // !!!

void main()
{
    mat4 m_matrix_trans = mat4(1.0, 0.0, 0.0, 0.0,
                               0.0, 1.0, 0.0, 0.0,
                               0.0, 0.0, 1.0, 0.0,
                               instancePosition.x, instancePosition.y, instancePosition.z, 1.0);

    mat4 m_matrix_inst = m_matrix_trans*m_matrix;
    mat4 pvm_matrix = p_matrix*v_matrix*m_matrix_inst;
    mat3 norm_matrix = mat3(transpose(inverse(m_matrix_inst)));

    gl_Position = pvm_matrix*vec4(position, 1.0);
    fragPos = m_matrix_inst*vec4(position, 1.0);
    fragNormal = normalize(norm_matrix*normal);
    fragTexCoor = textureCoor;
    fragColor = instanceColor;
}
