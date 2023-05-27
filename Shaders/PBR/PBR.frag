#version 420 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 texCoord;
in vec3 worldPos;
in vec3 normal;
in mat3 TBN;
in mat3 transposeModel;

layout(binding = 0) uniform sampler2D albedoTex;
layout(binding = 1) uniform sampler2D normalTex;
layout(binding = 2) uniform sampler2D metallicTex;
layout(binding = 3) uniform sampler2D roughnessTex;
layout(binding = 4) uniform sampler2D AOTex;
layout(binding = 5) uniform samplerCube irradianceMap;
layout(binding = 6) uniform samplerCube prefilterMap;
layout(binding = 7) uniform sampler2D   brdfLUT;

struct DirLight{
	vec3 direction;
	vec3 diffuse;

	float intensity;
};
struct PointLight {
	vec3 position;
	vec3 diffuse;
	
	float intensity;
};
struct SpotLight {
	vec3 position;
	vec3 direction;
	vec3 diffuse;

	float cutOff;
	float outerCutOff;
	
	float intensity;
};

#define NR_POINT_LIGHTS 1

uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;

uniform bool dirLightEnabled;
uniform bool pointLightEnabled;
uniform bool spotLightEnabled;

// IBL

uniform vec3 viewPos;

uniform bool iblEnabled;

uniform bool transformSRGB;
uniform bool bloomOn;

uniform bool useAlbedo;
uniform bool useNormalMap;
uniform bool useMetallic;
uniform bool useRoughness;
uniform bool useAmbientMap;

//Deferred
uniform bool deferredEnabled;
uniform int deferredState;

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

vec3 CalcDirLight(DirLight light, vec3 albedo, vec3 normal, float metallic, float roughness, float ao);
vec3 CalcPointLight(PointLight light, vec3 albedo, vec3 normal, float metallic, float roughness, float ao);
vec3 CalcSpotLight(SpotLight light, vec3 albedo, vec3 normal, float metallic, float roughness, float ao);
vec3 CalcAmbient(vec3 albedo, vec3 normal, float metallic, float roughness, float ao);
vec3 getNormalFromMap(){
    vec3 tangentNormal = texture(normalTex, texCoord).xyz * 2.0 - 1.0;

    //vec3 Q1  = dFdx(worldPos);
    //vec3 Q2  = dFdy(worldPos);
    //vec2 st1 = dFdx(texCoord);
    //vec2 st2 = dFdy(texCoord);
	//
    //vec3 N   = normalize(normal);
    //vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    //vec3 B  = -normalize(cross(N, T));
    //mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

vec3 viewDir;

// calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
// of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
vec3 F0 = vec3(0.04); 

// reflectance equation
vec3 Lo = vec3(0.0);

void main(){
	vec3 albedo;
	vec3 aNormal;
	float metallic;
	float roughness;
	float ao;

	if(useAlbedo) albedo = texture(albedoTex, texCoord).rgb;
	if(useMetallic) metallic = texture(metallicTex, texCoord).r;
	if(useRoughness) roughness = texture(roughnessTex, texCoord).r;
	else roughness = 1.f;

	if(useAmbientMap) ao = texture(AOTex, texCoord).r;
	else ao = 1.f;

	if(transformSRGB) albedo = pow(albedo, vec3(2.2f));

	if(!useNormalMap || texture(normalTex, texCoord).rgb == vec3(0.f) || TBN == mat3(0.f)) //If normalMap is empty or you cant transform a normal map to a normal vector just use the vertex normal vector
		aNormal = normal;
	else
		aNormal = getNormalFromMap();
	
	//normal = someNormal;
	viewDir = normalize(viewPos - worldPos);

	F0 = mix(F0, albedo, metallic);


	vec3 result;

	if(dirLightEnabled)
		result += CalcDirLight(dirLight, albedo, aNormal, metallic, roughness, ao);

	if(pointLightEnabled){
		for(int i = 0; i < NR_POINT_LIGHTS; i++){
			result += CalcPointLight(pointLights[i], albedo, aNormal, metallic, roughness, ao);
		}
	}

	if(spotLightEnabled)
		result += CalcSpotLight(spotLight, albedo, aNormal, metallic, roughness, ao);

	
	if(iblEnabled) result += CalcAmbient(albedo, aNormal, metallic, roughness, ao);

	FragColor = vec4(result, 1.0);

	//Bloom
	float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722)); //Transform into a luminance value
	if(brightness > 1.0 && bloomOn)
		BrightColor = vec4(FragColor.rgb, 1.0);
	else
		BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
vec3 CalcDirLight(DirLight light, vec3 albedo, vec3 normal, float metallic, float roughness, float ao){
	if(light.diffuse == vec3(0.f)) return vec3(0.f); //If empty just stop
	
	// calculate per-light radiance
	vec3 lightDir = -light.direction;
	vec3 halfwayDir = normalize(viewDir + lightDir);
	vec3 radiance = light.diffuse;

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
	Lo += (kD * albedo / PI + specular) * radiance * NdotL * light.intensity; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
	
	return Lo;
}
vec3 CalcPointLight(PointLight light, vec3 albedo, vec3 normal, float metallic, float roughness, float ao){
	if(light.diffuse == vec3(0.f)) return vec3(0.f); //If empty just stop
	
	// calculate per-light radiance
	vec3 lightDir = normalize(light.position - worldPos);
	vec3 halfwayDir = normalize(viewDir + lightDir);
	float distance = length(light.position - worldPos);
	float attenuation = 1.0 / (distance * distance);
	vec3 radiance = light.diffuse * attenuation;

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
	Lo += (kD * albedo / PI + specular) * radiance * NdotL * light.intensity; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
	
	return Lo;//texture(material.normal, texCoord).rgb;//(TBN * (texture(material.normal, texCoord).rgb*2.f-1.f))*0.5f + 0.5f;
}
vec3 CalcSpotLight(SpotLight light, vec3 albedo, vec3 normal, float metallic, float roughness, float ao){
	if(light.diffuse == vec3(0.f)) return vec3(0.f); //If empty just stop
	
	// calculate per-light radiance
	vec3 lightDir = normalize(light.position - worldPos);
	vec3 halfwayDir = normalize(viewDir + lightDir);
	float distance = length(light.position - worldPos);
	float attenuation = 1.0 / (distance * distance);
	vec3 radiance = light.diffuse * attenuation;

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
	Lo += (kD * albedo / PI + specular) * radiance * NdotL * light.intensity; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
	
	//Spotlight range
	float theta = dot(lightDir, normalize(-light.direction)); 
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

	Lo *= intensity;

	return Lo;
}
vec3 CalcAmbient(vec3 albedo, vec3 normal, float metallic, float roughness, float ao){
	vec3 reflectDir = reflect(-viewDir, normal);
	vec3 F        = fresnelSchlickRoughness(max(dot(normal, viewDir), 0.0), F0, roughness);
	
	vec3 kS = F;
	vec3 kD = 1.0 - kS;
	kD *= 1.0 - metallic;

	vec3 irradiance = texture(irradianceMap, normal).rgb;
	vec3 diffuse      = irradiance * albedo;

	const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, reflectDir,  roughness * MAX_REFLECTION_LOD).rgb;
	vec2 brdf  = texture(brdfLUT, vec2(max(dot(normal, viewDir), 0.0), roughness)).rg;
	vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

	// ambient lighting (we now use IBL as the ambient term)
	vec3 ambient;

	ambient = (kD * diffuse + specular) * ao;
	
	return ambient;
}