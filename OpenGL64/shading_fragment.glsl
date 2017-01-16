#version 450

// shading_fragment

in vec2 fragTexCoor;

out vec4 color;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColor;
uniform int width;
uniform int height;

struct Light
{
    vec3 pos;
    vec3 color;
};

uniform Light light;
uniform vec3 eyePos;

uniform int slider;
uniform float edge;


void main()
{
  vec4 p = texture(gPosition, fragTexCoor);
  if (p.w != 0.0)
    discard;

  vec3 fPosition = p.xyz;
  vec3 fNormal   = texture(gNormal, fragTexCoor).xyz;

  vec4 c = texture(gColor, fragTexCoor);
  vec3 fColor    = c.rgb;
  float fAmbient = c.a;

  // ambient
  vec3 ambient = fColor * fAmbient;

  // diffuse
  vec3 lightDir = normalize(light.pos - fPosition.xyz);
  vec3 diffuse = max(dot(fNormal, lightDir), 0.0)*light.color*fColor;

  // specular
  float specular_strength = 0.5;
  int specular_pow = 20;
  vec3 viewDir = normalize(eyePos - fPosition.xyz);
  vec3 reflDir = reflect(-lightDir, fNormal);
  vec3 specular = specular_strength*pow(max(dot(viewDir, reflDir), 0.0), specular_pow)*light.color;

  color = vec4(ambient + diffuse + specular, 1.0);

  if (slider < int(gl_FragCoord.x))
    color = vec4(abs(fNormal), 1.0);

  if (edge < 5.0)
  {
    vec2 texelSize = vec2(1.0/width, 1.0/height);

    // Sobel edge detection
    vec3 sumx = vec3(0.0);
    sumx += (texture(gNormal, fragTexCoor + texelSize*vec2(-1, -1)).xyz) * -1;
    sumx += (texture(gNormal, fragTexCoor + texelSize*vec2( 0, -1)).xyz) * -2;
    sumx += (texture(gNormal, fragTexCoor + texelSize*vec2( 1, -1)).xyz) * -1;

    sumx += (texture(gNormal, fragTexCoor + texelSize*vec2(-1,  1)).xyz) *  1;
    sumx += (texture(gNormal, fragTexCoor + texelSize*vec2( 0,  1)).xyz) *  2;
    sumx += (texture(gNormal, fragTexCoor + texelSize*vec2( 1,  1)).xyz) *  1;

    vec3 sumy = vec3(0.0);
    sumy += (texture(gNormal, fragTexCoor + texelSize*vec2(-1, -1)).xyz) * -1;
    sumy += (texture(gNormal, fragTexCoor + texelSize*vec2(-1,  0)).xyz) * -2;
    sumy += (texture(gNormal, fragTexCoor + texelSize*vec2(-1,  1)).xyz) * -1;

    sumy += (texture(gNormal, fragTexCoor + texelSize*vec2( 1, -1)).xyz) *  1;
    sumy += (texture(gNormal, fragTexCoor + texelSize*vec2( 1,  0)).xyz) *  2;
    sumy += (texture(gNormal, fragTexCoor + texelSize*vec2( 1,  1)).xyz) *  1;

    vec3 sum = sumx*sumx + sumy*sumy;
    if (sum.x > edge || sum.y > edge || sum.z > edge) color = vec4(0.0, 0.0, 0.0, 1);


/*
    // Roberts edge detection
    vec3 sumx = vec3(0.0);
    sumx += (texture(gNormal, fragTexCoor + texelSize*vec2( 0,  0)).xyz) *  1;
    sumx += (texture(gNormal, fragTexCoor + texelSize*vec2( 1,  1)).xyz) * -1;

    vec3 sumy = vec3(0.0);
    sumy += (texture(gNormal, fragTexCoor + texelSize*vec2( 1,  0)).xyz) *  1;
    sumy += (texture(gNormal, fragTexCoor + texelSize*vec2( 0,  1)).xyz) * -1;

    vec3 sum = sumx*sumx + sumy*sumy;
    if (sum.x > edge || sum.y > edge || sum.z > edge) color = vec4(0.0, 0.0, 0.0, 1);
*/
  }

//color += vec4(0.6, 0.6, 0.6, 0.0);
}
