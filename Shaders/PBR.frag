#version 330 core
out vec4 FragColor;
in vec2 texCoord;
in vec3 fragPos;
in vec3 someNormal;
in mat3 TBN;

// material parameters

struct Material {
	sampler2D albedo;
	sampler2D normal;
	sampler2D metallic;
	sampler2D roughness;
	sampler2D AO; //Ambient occlusion
};

// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D   brdfLUT;

// lights
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

uniform vec3 viewPos;

uniform Material mat;

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness){
	float a = roughness*roughness;
	float a2 = a*a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH*NdotH;

	float nom   = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return nom / denom;
}
float GeometrySchlickGGX(float NdotV, float roughness){
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;

	float nom   = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness){
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}
vec3 fresnelSchlick(float cosTheta, vec3 F0){
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness){
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}   

void main()
{		
	vec3 viewDir = normalize(viewPos - fragPos);

	vec3 albedo = pow(texture(mat.albedo, texCoord).rgb, vec3(2.2f));
	vec3 normal = texture(mat.normal, texCoord).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	normal = TBN * normal;
	float metallic = texture(mat.metallic, texCoord).r;
	float roughness = texture(mat.roughness, texCoord).r;
	float ao = texture(mat.AO, texCoord).r;

	// calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
	// of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, albedo, metallic);

	// reflectance equation
	vec3 Lo = vec3(0.0);
	for(int i = 0; i < 4; ++i) 
	{
		// calculate per-light radiance
		vec3 lightDir = normalize(lightPositions[i] - fragPos);
		vec3 halfwayDir = normalize(viewDir + lightDir);
		float distance = length(lightPositions[i] - fragPos);
		float attenuation = 1.0 / (distance * distance);
		vec3 radiance = lightColors[i] * attenuation;

		// Cook-Torrance BRDF
		float NDF = DistributionGGX(normal, halfwayDir, roughness);   
		float G   = GeometrySmith(normal, viewDir, lightDir, roughness);    
		vec3 F    = fresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), F0);        
		
		vec3 numerator    = NDF * G * F;
		float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
		vec3 specular = numerator / denominator;
		
		 // kS is equal to Fresnel
		vec3 kS = F;
		// for energy conservation, the diffuse and specular light can't
		// be above 1.0 (unless the surface emits light); to preserve this
		// relationship the diffuse component (kD) should equal 1.0 - kS.
		vec3 kD = vec3(1.0) - kS;
		// multiply kD by the inverse metalness such that only non-metals 
		// have diffuse lighting, or a linear blend if partly metal (pure metals
		// have no diffuse light).
		kD *= 1.0 - metallic;	                
			
		// scale light by NdotL
		float NdotL = max(dot(normal, lightDir), 0.0);

		// add to outgoing radiance Lo
		Lo += (kD * albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
	}   
	
	vec3 R = reflect(-viewDir, normal);

	vec3 F        = fresnelSchlickRoughness(max(dot(normal, viewDir), 0.0), F0, roughness);
	
	vec3 kS = F;
	vec3 kD = 1.0 - kS;
	kD *= 1.0 - metallic;

	vec3 irradiance = texture(irradianceMap, normal).rgb;
	vec3 diffuse      = irradiance * albedo;

	const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;
	vec2 brdf  = texture(brdfLUT, vec2(max(dot(normal, viewDir), 0.0), roughness)).rg;
	vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

	// ambient lighting (we now use IBL as the ambient term)

	vec3 ambient = (kD * diffuse + specular) * ao;
	
	vec3 color = ambient + Lo;

	// HDR tonemapping
	color = color / (color + vec3(1.0));
	// gamma correct
	color = pow(color, vec3(1.0/2.2)); 

	FragColor = vec4(color, 1.0);
	//(1-x)*(1-y) = 1 - y - x +xy
}
