#version 450

struct Light
{
    vec3 pos;
    vec3 color;
};

in vec3 position;
in vec3 normal;
in vec2 textureCoor;
in vec3 tgnU;
in vec3 tgnV;

uniform mat4 p_matrix;
uniform mat4 v_matrix;
uniform mat4 m_matrix;
uniform mat3 norm_matrix;
uniform Light light;   // !!!
uniform vec3 eyePos;   // !!!

out vec2 fragTexCoor;
out vec3 fragTanEyePos;   // !!!
out vec3 fragTanLightPos; // !!!
out vec3 fragTanFragPos;  // !!!
out vec3 fragTanNormal;   // !!!


void main()
{
    mat4 pvm_matrix = p_matrix*v_matrix*m_matrix;

    gl_Position = pvm_matrix*vec4(position, 1.0);

    vec3 fragPos = vec3(m_matrix*vec4(position, 1.0));

    vec3 T = normalize(norm_matrix * tgnU);
    vec3 B = normalize(norm_matrix * tgnV);
    vec3 N = normalize(norm_matrix * normal);
    mat3 TBN = transpose(mat3(T, B, N));

    fragTanLightPos = TBN * light.pos;
    fragTanEyePos   = TBN * eyePos;
    fragTanFragPos  = TBN * fragPos;
    fragTanNormal   = TBN * N;

    fragTexCoor = textureCoor;
}
