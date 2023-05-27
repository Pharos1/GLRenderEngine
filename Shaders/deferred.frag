#version 420 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec2 texCoord;
in vec3 worldPos;
in vec3 normal;

in mat3 TBN;

layout(binding = 0) uniform sampler2D albedoTex;
layout(binding = 1) uniform sampler2D normalTex;
layout(binding = 2) uniform sampler2D metallicTex;

void main(){
    // store the fragment position vector in the first gbuffer texture
    gPosition = worldPos;
    // also store the per-fragment normals into the gbuffer
    if(texture(normalTex, texCoord).rgb == vec3(0.f) || TBN == mat3(0.f))
        gNormal = normal;
    else
        gNormal = normalize(TBN * (texture(normalTex, texCoord).rgb * 2.f - 1.f));
    // and the diffuse per-fragment color
    gAlbedoSpec.rgb = texture(albedoTex, texCoord).rgb;
    // store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec.a = texture(metallicTex, texCoord).r;
}