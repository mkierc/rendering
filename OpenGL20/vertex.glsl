#version 450

in vec3 position;
in vec2 textureCoor;
in vec3 normal;
in vec3 tgnU;
in vec3 tgnV;

uniform mat4 p_matrix;
uniform mat4 v_matrix;
uniform mat4 m_matrix;
uniform mat3 norm_matrix;

out vec4 fragPos;
out vec2 fragTexCoor;
out mat3 fragTBN;

void main()
{
    mat4 pvm_matrix = p_matrix*v_matrix*m_matrix;

    gl_Position = pvm_matrix*vec4(position, 1.0);
    fragPos = m_matrix*vec4(position, 1.0);

    vec3 T = normalize(norm_matrix * tgnU);
    vec3 B = normalize(norm_matrix * tgnV);
    vec3 N = normalize(norm_matrix * normal);
    fragTBN = mat3(T, B, N);

    fragTexCoor = textureCoor;
}
