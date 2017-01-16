#version 450

in vec2 fragTexCoor;

uniform sampler2D tex;
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
