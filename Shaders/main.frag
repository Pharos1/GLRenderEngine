#version 420 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

/*Bloom*/
uniform bool bloomOn;

in vec3 normal;
in vec3 worldPos;
in vec2 texCoord;
in vec4 fragPosLightSpace;

in mat3 TBN;

layout(binding = 0) uniform sampler2D albedoTex;
layout(binding = 1) uniform sampler2D normalTex;
layout(binding = 2) uniform sampler2D metallicTex;
layout(binding = 3) uniform sampler2D roughnessTex;
layout(binding = 4) uniform sampler2D AOTex;
uniform float shininessExponent;

layout(binding = 5) uniform sampler2D positionBuffer;
layout(binding = 6) uniform sampler2D normalBuffer;
layout(binding = 7) uniform sampler2D albedoBuffer;


struct DirLight {
	vec3 direction;

	//vec3 ambient;
	vec3 diffuse;
	//vec3 specular;
};
struct PointLight {
	vec3 position;
	
	float constant;
	float linear;
	float quadratic;

	//vec3 ambient;
	vec3 diffuse;
	//vec3 specular;
};
struct SpotLight {
	vec3 position;
	vec3 direction;

	float constant;
	float linear;
	float quadratic;

	//vec3 ambient;
	vec3 diffuse;
	//vec3 specular;

	float cutOff;
	float outerCutOff;
};
#define NR_POINT_LIGHTS 1

uniform vec3 viewPos;

uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;

uniform bool dirLightEnabled;
uniform bool pointLightEnabled;
uniform bool spotLightEnabled;

//Deferred
uniform bool deferredEnabled;
uniform int deferredState = 4;

uniform bool transformSRGB;

//Global variables
vec3 viewDir;

//Methods
float spec(vec3 lightDir, vec3 normal);
vec3 CalcDirLight(DirLight light, vec3 normal, vec2 texCoord, float shininess, vec3 fragPos, vec3 albedo);
vec3 CalcPointLight(PointLight light, vec3 normal, vec2 texCoord, float shininess, vec3 fragPos, vec3 albedo);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec2 texCoord, float shininess, vec3 fragPos, vec3 albedo);
vec3 getNormalFromMap(){
    vec3 tangentNormal = texture(normalTex, texCoord).xyz * 2.0 - 1.0;
    return normalize(TBN * tangentNormal);
}

//uniform sampler2D shadowMap; //SHADOWS

//uniform samplerCube depthMap; //SHADOWS(cubemap)
//uniform float far_plane; //SHADOWS(cubemap)

//uniform float height_scale; //Displacement map

/*SHADOWS
float ShadowCalculation(vec4 fragPosLightSpace, vec3 lightDir){
	// perform perspective divide
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	// transform to [0,1] range
	projCoords = projCoords * 0.5 + 0.5;
	// get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
	float closestDepth = texture(shadowMap, projCoords.xy).r; 
	// get depth of current fragment from light's perspective
	float currentDepth = projCoords.z;
	// check whether current frag pos is in shadow
	//float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;
	
	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
	//float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
	
	//PCF
	float shadow = 0.f;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}    
	}
	shadow /= 9.0;

	if(projCoords.z > 1.0)
		shadow = 0.0;
	return shadow;
}
*/
/*SHADOWS(cubemap)
float ShadowCalculation(vec3 lightPos)
{
	// get vector between fragment position and light position
	vec3 fragToLight = fragPos - lightPos;
	// use the light to fragment vector to sample from the depth map    
	float closestDepth = texture(depthMap, fragToLight).r;
	// it is currently in linear range between [0,1]. Re-transform back to original value
	closestDepth *= far_plane;
	// now get current linear depth as the length between the fragment and light position
	float currentDepth = length(fragToLight);
	// now test for shadows

	//PCF
	vec3 sampleOffsetDirections[20] = vec3[]
	(
	vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
	vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
	vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
	vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
	vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
	);   

	float shadow = 0.0;
	float bias   = 0.15;
	int samples  = 20;
	float viewDistance = length(viewPos - fragPos);
	float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;  
	for(int i = 0; i < samples; ++i)
	{
		float closestDepth = texture(depthMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
		closestDepth *= far_plane;   // undo mapping [0;1]
		if(currentDepth - bias > closestDepth)
			shadow += 1.0;
	}
	shadow /= float(samples); 

	return shadow;
}
*/

/*
vec2 ParallaxMapping(vec2 texCoord, vec3 viewDir){
	//float height = texture(material.texture_displacement1, texCoord).r;
	//vec2 p = viewDir.xy / viewDir.z * (height * height_scale);
	//
	//return texCoord - p;

	 // number of depth layers

	const float minLayers = 8.0;
	const float maxLayers = 32.0;
	float numLayers = mix(maxLayers, minLayers, max(dot(vec3(0.0, 0.0, 1.0), viewDir), 0.0)); 
	// calculate the size of each layer
	float layerDepth = 1.0 / numLayers;
	// depth of current layer
	float currentLayerDepth = 0.0;
	// the amount to shift the texture coordinates per layer (from vector P)
	vec2 P = viewDir.xy * height_scale; 
	vec2 deltaTexCoords = P / numLayers;

	// get initial values
	vec2  currentTexCoords     = texCoord;
	float currentDepthMapValue = texture(material.texture_displacement1, currentTexCoords).r;
	
	while(currentLayerDepth < currentDepthMapValue)
	{
		// shift texture coordinates along direction of P
		currentTexCoords -= deltaTexCoords;
		// get depthmap value at current texture coordinates
		currentDepthMapValue = texture(material.texture_displacement1, currentTexCoords).r;  
		// get depth of next layer
		currentLayerDepth += layerDepth;  
	}
	// get texture coordinates before collision (reverse operations)
	vec2 prevTexCoords = currentTexCoords + deltaTexCoords;
	
	// get depth after and before collision for linear interpolation
	float afterDepth  = currentDepthMapValue - currentLayerDepth;
	float beforeDepth = texture(material.texture_displacement1, prevTexCoords).r - currentLayerDepth + layerDepth;
	
	// interpolation of texture coordinates
	float weight = afterDepth / (afterDepth - beforeDepth);
	vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);
	
	return finalTexCoords;
}
*/


#define ambientConst 0.1f
#define specularConst 0.3f

void main(){
	//Set global variables
	vec3 aWorldPos;
	vec3 albedo;
	float shininess;
	vec3 aNormal;

	if(deferredEnabled){
		aWorldPos = texture(positionBuffer, texCoord).rgb;
		albedo = texture(albedoBuffer, texCoord).rgb;
		shininess = texture(albedoBuffer, texCoord).a;
		aNormal = texture(normalBuffer, texCoord).rgb;

		if(aNormal == vec3(0.f)) discard; //If empty just discard the pixel
	}
	else{
		aWorldPos = worldPos;
		albedo = texture(albedoTex, texCoord).rgb;
		shininess = texture(metallicTex, texCoord).r;
		//aNormal = texture(material.texture_normal1, texCoord).rgb;
		//aNormal = normalize(aNormal * 2.0 - 1.0);
		if(texture(normalTex, texCoord).rgb == vec3(0.f) || TBN == mat3(0.f)) //If normalMap is empty or you cant transform a normal map to a normal vector just use the vertex normal vector
			aNormal = normal;
		else
			aNormal = getNormalFromMap();
	}
	if(transformSRGB) albedo = pow(albedo, vec3(2.2f));

	viewDir = normalize(viewPos - aWorldPos);
	

	//vec2 updatedTexCoord = texCoord;//ParallaxMapping(texCoord,  viewDir); //Paralax Mapping
	//if(texCoord.x > 1.0 || texCoord.y > 1.0 || texCoord.x < 0.0 || texCoord.y < 0.0) //Paralax Mapping
	//	discard; //Paralax Mapping

	//Combine lights
	vec3 result = vec3(0.f);

	if(dirLightEnabled) result += CalcDirLight(dirLight, aNormal, texCoord, shininess, aWorldPos, albedo);

	if(pointLightEnabled){
		for(int i = 0; i < NR_POINT_LIGHTS; i++)
			result += CalcPointLight(pointLights[i], aNormal, texCoord, shininess, aWorldPos, albedo);
	}
	
	if(spotLightEnabled) result += CalcSpotLight(spotLight, aNormal, texCoord, shininess, aWorldPos, albedo);
	
	//if(texColor.a < 0.1f) discard; //Transparency

	FragColor = vec4(result, 1.f);

	if(deferredEnabled){
		if(deferredState == 0) FragColor = vec4(aWorldPos, 1.f);
		if(deferredState == 1) FragColor = vec4(aNormal, 1.f);
		if(deferredState == 2) FragColor = vec4(albedo, 1.f);
		if(deferredState == 3) FragColor = vec4(shininess, shininess, shininess, 1.f);
		if(deferredState == 4) FragColor = vec4(result, 1.f);
	}

	/*Bloom*/
	// check whether fragment output is higher than threshold, if so output as brightness color
	float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722)); //Transform into a luminance value
	if(brightness > 1.0 && bloomOn)
		BrightColor = vec4(FragColor.rgb, 1.0);
	else
		BrightColor = vec4(0.0, 0.0, 0.0, 1.0);

	/*SHADOWS
	//Set important variables
	DirLight light = dirLight;
	
	vec3 lightDir = normalize(-light.direction);
	
	vec3 ambient, diffuse, specular;
	
	//Ambient lighting
	ambient = light.ambient;
	
	//Diffuse lighting
	float diff = max(dot(normal, lightDir), 0.0);
	diffuse = light.diffuse * diff;
	
	//Specular lighting
	specular = light.specular * spec(lightDir);

	float shadow = ShadowCalculation(fragPosLightSpace, lightDir);
	vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * texture(material.texture_diffuse1, texCoord).rgb;
	
	FragColor = vec4(lighting, 1.0);
	*/

	/*SHADOWS(cubemap)
	//Set important variables
	PointLight light = pointLights[0];
	
	//Set important variables
	vec3 lightDir = lightDir(light.position);

	vec3 ambient, diffuse, specular;
	ambient = diffuse = texture(material.texture_diffuse1, texCoord).rgb;
	specular = texture(material.texture_specular1, texCoord).rgb;

	//Ambient lighting
	ambient *= light.ambient;

	//Diffuse lighting
	float diff = max(dot(lightDir, normal), 0.0);
	diffuse *= light.diffuse * diff;

	//Attenuation
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
	
	//Specular lighting
	specular *= light.specular * spec(lightDir);

	float shadow = ShadowCalculation(light.position);
	vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * attenuation * texture(material.texture_diffuse1, texCoord).rgb;
	FragColor = vec4(lighting, 1.0);
	*/
}

//vec3 lightDir(vec3 lightPos){ return normalize(lightPos - fragPos); }
float spec(vec3 lightDir, vec3 aNormal){
	vec3 halfwayDir = normalize(lightDir + viewDir);

	return pow(max(dot(aNormal, halfwayDir), 0.0), shininessExponent);
}
vec3 CalcDirLight(DirLight light, vec3 normal, vec2 texCoord, float shininess, vec3 fragPos, vec3 albedo){
	if(light.diffuse == vec3(0.f)) return vec3(0.f); //If empty just stop
	
	//Set important variables
	vec3 lightDir = normalize(-light.direction);

	vec3 ambient = albedo, diffuse = albedo;
	float specular = shininess;

	//Ambient lighting
	ambient *= ambientConst;//light.ambient;

	//Diffuse lighting
	float diff = max(dot(normal, lightDir), 0.0);
	diffuse *= light.diffuse * diff;

	//Specular lighting
	specular *= specularConst/*light.specular*/*spec(lightDir, normal);

	return (ambient + diffuse + specular);
}
vec3 CalcPointLight(PointLight light, vec3 normal, vec2 texCoord, float shininess, vec3 fragPos, vec3 albedo){
	if(light.diffuse == vec3(0.f)) return vec3(0.f); //If empty just stop
	
	//Set important variables
	vec3 lightDir = normalize(light.position - fragPos);

	vec3 ambient = albedo, diffuse = albedo;
	float specular = shininess;

	//Ambient lighting
	ambient *= ambientConst;//light.ambient;

	//Diffuse lighting
	float diff = max(dot(lightDir, normal), 0.0);
	diffuse *= light.diffuse * diff;

	//Attenuation
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (distance * distance);//(light.constant + light.linear * distance + light.quadratic * (distance * distance));
	
	//Specular lighting
	specular *= specularConst/*light.specular*/ * spec(lightDir, normal);

	return (ambient + diffuse + specular) * attenuation;
}
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec2 texCoord, float shininess, vec3 fragPos, vec3 albedo){
	if(light.diffuse == vec3(0.f)) return vec3(0.f); //If empty just stop
	
	//Set important variables
	vec3 lightDir = normalize(light.position - fragPos);
	
	vec3 ambient = albedo, diffuse = albedo;
	float specular = shininess;

	//Ambient lighting
	//ambient *= ambientConst;//light.ambient; //SpotLight doesn't have ambient lighting

	//Diffuse lighting
	float diff = max(dot(normal, lightDir), 0.0);
	diffuse *= light.diffuse * diff;

	//Specular lighting
	specular *= specularConst/*light.specular*/ * spec(lightDir, normal);

	//Attenuation
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (distance * distance);//(light.constant + light.linear * distance + light.quadratic * (distance * distance));    
	
	//Spotlight range
	float theta = dot(lightDir, normalize(-light.direction)); 
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

	return (ambient + diffuse + specular) * attenuation * intensity;
}