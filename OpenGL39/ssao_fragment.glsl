#version 450

// ssao_fragment

in vec2 fragTexCoor;

uniform sampler2D gPosition;
uniform sampler2D gDepth;
uniform sampler2D gNormal;

uniform mat4 p_matrix;
uniform mat4 v_matrix;

const int ssao_kernel_size = 64;

uniform vec3 ssao_kernel[ssao_kernel_size];
uniform float near;
uniform float far;
uniform float ssao_radius;

out float color;


float LinearizeDepth(float depth)
{
  return (2.0 * near * far) / (far + near - depth * (far - near));
}


void main ()
{
    vec3 fragPos = texture(gPosition, fragTexCoor).xyz;
         fragPos = (v_matrix*vec4(fragPos, 1.0)).xyz;
    vec4 fragPosPersp = p_matrix*vec4(fragPos, 1.0);
    fragPosPersp.xyz /= fragPosPersp.w;
    float fragPosPersp_lz = LinearizeDepth(fragPosPersp.z);

    vec3 randomVec = vec3(-1.0, 0.0, 0.0);
    vec3 normal = texture(gNormal, fragTexCoor).rgb;
         normal = (v_matrix*vec4(normal.xyz, 1.0)).xyz;
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for (int i = 0; i < ssao_kernel_size; i++)
    {
        vec3 ssaoVec = ssao_kernel[i] * ssao_radius;
        ssaoVec.z = abs(ssaoVec.z) + ssao_radius*0.2;
        ssaoVec = TBN * ssaoVec;
        vec4 ssaoPoint = p_matrix * vec4(fragPos + ssaoVec, 1.0);

        ssaoPoint.xyz /= ssaoPoint.w;
        float ssaoPoint_lz = LinearizeDepth(ssaoPoint.z);

        float depth = texture(gDepth, ssaoPoint.xy*0.5 + 0.5).x;
        float range_check = smoothstep(0.0, 1.0, ssao_radius/abs(fragPosPersp_lz - depth));

        occlusion += (depth < ssaoPoint_lz ? 1.0 : 0.0) * range_check;
    }

    color = 1 - occlusion / ssao_kernel_size;
}

