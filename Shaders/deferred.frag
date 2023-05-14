#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec2 texCoord;
in vec3 worldPos;
in vec3 normal;

in mat3 TBN;

struct Material {
	sampler2D albedo;
	sampler2D normal;
	sampler2D metallic;
	//sampler2D roughness;
	//sampler2D AO;

	//float shininess;
};

uniform Material material;

void main(){
    // store the fragment position vector in the first gbuffer texture
    gPosition = worldPos;
    // also store the per-fragment normals into the gbuffer
    if(texture(material.normal, texCoord).rgb == vec3(0.f) || TBN == mat3(0.f))
        gNormal = normal;
    else
        gNormal = normalize(TBN * (texture(material.normal, texCoord).rgb * 2.f - 1.f));
    // and the diffuse per-fragment color
    gAlbedoSpec.rgb = texture(material.albedo, texCoord).rgb;
    // store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec.a = texture(material.metallic, texCoord).r;
}