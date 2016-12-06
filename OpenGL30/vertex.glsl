#version 450

in vec3 position;
in vec3 normal;
in vec2 textureCoor;

uniform mat4 p_matrix;
uniform mat4 v_matrix;
uniform mat4 m_matrix;
uniform mat3 norm_matrix;
uniform mat4 light_matrix; // !!!

out vec3 fragNormal;
out vec4 fragPos;
out vec2 fragTexCoor;
out vec4 fragPosLight; // !!!

void main()
{
    mat4 pvm_matrix = p_matrix*v_matrix*m_matrix;

    gl_Position = pvm_matrix*vec4(position, 1.0);
    fragPos = m_matrix*vec4(position, 1.0);

    fragNormal = normalize(norm_matrix*normal);

    fragTexCoor = textureCoor;

    fragPosLight = light_matrix*m_matrix*vec4(position, 1.0); // !!!
}
