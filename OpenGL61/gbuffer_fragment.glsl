#version 450

// gbuffer_fragment

in vec3 fragNormal;
in vec3 fragPos;
in vec2 fragTexCoor;

uniform sampler2D textureBitmap;
uniform float ambient;

uniform int object_id;
uniform int is_selected;

layout (location = 0) out vec4 gPosition; // XYZ + presence
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gColor;    // RGB + Ambient
layout (location = 3) out float gId;      // ID

void main()
{
    gPosition = vec4(fragPos, 0.0);      // pozycja + istnienie
    gNormal   = normalize(fragNormal);   // normalna
    gColor    = vec4(texture(textureBitmap, fragTexCoor).rgb, ambient); // kolor + ambient
    if (is_selected == 1)
     {
      gColor.r = 0;
      gColor.g = 0.5;
      gColor.b = 0;
      gColor.a = 0.5;
     }
    gId       = object_id;
}
