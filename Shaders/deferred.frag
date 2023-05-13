#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec2 texCoord;
in vec3 worldPos;
in vec3 normal;

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_diffuse2;
    sampler2D texture_diffuse3;

    sampler2D texture_specular1;
    sampler2D texture_specular2;

    sampler2D texture_normal1;

    sampler2D texture_displacement1;

    float shininess;
};

uniform Material material;

void main()
{    
    // store the fragment position vector in the first gbuffer texture
    gPosition = worldPos;
    // also store the per-fragment normals into the gbuffer
    gNormal = normal;
    // and the diffuse per-fragment color
    gAlbedoSpec.rgb = texture(material.texture_diffuse1, texCoord).rgb;
    // store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec.a = texture(material.texture_specular1, texCoord).r;
}  