#version 450

in vec3 fragTexCoor;

uniform samplerCube textureSkybox;

uniform int slider;

out vec4 color;

void main()
{
    if (int(gl_FragCoord.x) < slider)
        discard;
    else
        color = texture(textureSkybox, fragTexCoor);
}
