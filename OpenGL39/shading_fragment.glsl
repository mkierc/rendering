#version 450

// shading_fragment

in vec2 fragTexCoor;

out vec4 color;

uniform sampler2D gPosition;  // XYZ + presence
uniform sampler2D gNormal;    // XYZ + specular
uniform sampler2D gColor;     // RGB + ambient
uniform sampler2D gSSAO;

struct Light
{
    vec3 pos;
    vec3 color;
};

uniform Light light;
uniform vec3 eyePos;

uniform int slider;


void main()
{
  vec4 p = texture(gPosition, fragTexCoor);
  if (p.w != 0.0)
   discard;

  vec3 fPosition = p.xyz;
  p = texture(gNormal, fragTexCoor);
  vec3 fNormal   = p.xyz;
  float specular_strength = p.w;
  vec3 fColor    = texture(gColor,    fragTexCoor).rgb;
  float fAmbient = texture(gColor,    fragTexCoor).a;

  // ambient
  vec3 ambient = fColor * fAmbient;

  // diffuse
  vec3 lightDir = normalize(light.pos - fPosition.xyz);
  vec3 diffuse = max(dot(fNormal, lightDir), 0.0)*light.color*fColor;

  // specular
  int specular_pow = 20;
  vec3 viewDir = normalize(eyePos - fPosition.xyz);
  vec3 reflDir = reflect(-lightDir, fNormal);
  vec3 specular = specular_strength*pow(max(dot(viewDir, reflDir), 0.0), specular_pow)*light.color;

  if (int(gl_FragCoord.x) >= slider)
    color = vec4(ambient + diffuse + specular, 1.0) * texture(gSSAO, fragTexCoor).x;
  else
    color = vec4(ambient + diffuse + specular, 1.0);

//  color = color*0.0001 + texture(gSSAO, fragTexCoor).rrra;
}
