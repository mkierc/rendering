#version 450

// gbuffer_fragment

in vec3 fragNormal;
in vec3 fragPos;
in vec2 fragTexCoor;

uniform sampler2D textureBitmap;
uniform float ambient;
uniform float specular;
uniform float near;
uniform float far;


layout (location = 0) out vec4 gPosition; // XYZ + presence
layout (location = 1) out vec4 gNormal;   // XYZ + specular
layout (location = 2) out vec4 gColor;    // RGB + ambient
layout (location = 3) out float gDepth;


float LinearizeDepth(float depth)
{
  // (0, 1) -> (near, far)
  float z = depth * 2.0 - 1.0;
  return (2.0 * near * far) / (far + near - z * (far - near));
}


void main()
{
    gPosition = vec4(fragPos, 0.0);
    gNormal   = vec4(normalize(fragNormal), specular);
    gColor    = vec4(texture(textureBitmap, fragTexCoor).rgb, ambient);
    gDepth    = LinearizeDepth(gl_FragCoord.z);
}
