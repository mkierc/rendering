#version 450

in vec3 position;
in vec2 texCoor;

uniform mat4 pv_matrix;
uniform float layer;

out vec3 fragTexCoor;

void main()
{
    gl_Position = vec4(position, 1.0);

    vec3 t = vec3(texCoor, layer);

    fragTexCoor = (pv_matrix*vec4(t, 1)).xyz;
}
