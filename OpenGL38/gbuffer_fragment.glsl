#version 450

// gbuffer_fragment

in mat3 fragTBN;
in vec3 fragPos;
in vec2 fragTexCoor;

uniform sampler2D textureBitmap;
uniform sampler2D textureNormal;
uniform float ambient;

layout (location = 0) out vec4 gPosition; // XYZ + presence
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gColor;    // RGB + Ambient


void main()
{
    gPosition = vec4(fragPos, 0.0);      // pozycja + istnienie
    gColor    = vec4(texture(textureBitmap, fragTexCoor).rgb, ambient); // kolor + ambient
    vec3 texNormal = texture(textureNormal, fragTexCoor).rgb * 2.0 - 1.0;

    gNormal = normalize(fragTBN*texNormal);
}
