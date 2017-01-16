#version 450

in vec3 fragTexCoor;

uniform sampler3D tex;

uniform float alpha;
uniform float treshold;

out vec4 color;


void main()
{
    color = texture(tex, fragTexCoor);

    if (color.a < treshold)
        discard;

    color.a *= alpha;
}
