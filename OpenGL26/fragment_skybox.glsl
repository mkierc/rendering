#version 450

in vec3 fragTexCoor;

uniform samplerCube textureSkybox;

out vec4 color;

void main()
{
    color = texture(textureSkybox, fragTexCoor);
}
