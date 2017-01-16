#version 450

in vec2 fragTexCoor;

uniform sampler2D tex1;
uniform sampler2D tex2;

uniform float alpha;
uniform float treshold;
uniform float tex_z;

out vec4 color;

void main()
{
    vec4 color1 = texture(tex1, fragTexCoor);
    vec4 color2 = texture(tex2, fragTexCoor);
    color = mix(color1, color2, tex_z);

    if (color.a < treshold)
        discard;

    color.a *= alpha;
}
