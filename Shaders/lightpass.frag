/*#version 330 core
out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D positionBuffer;
uniform sampler2D normalBuffer;
uniform sampler2D albedoBuffer;
//uniform sampler2D specularBuffer;

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_normal1;
    sampler2D texture_displacement1;

    float shininess;
};
struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
struct SpotLight {
    vec3 position;
    vec3 direction;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float cutOff;
    float outerCutOff;
};
#define NR_POINT_LIGHTS 4


uniform vec3 viewPos;

uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;

//Global variables
vec3 viewDir;

uniform Material material;

//Methods
float spec(vec3 lightDir, vec3 normal);
vec3 CalcDirLight(DirLight light, vec3 normal, vec2 texCoord, float shininess, vec3 fragPos, vec3 albedo);
vec3 CalcPointLight(PointLight light, vec3 normal, vec2 texCoord, float shininess, vec3 fragPos, vec3 albedo);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec2 texCoord, float shininess, vec3 fragPos, vec3 albedo);

void main(){
    vec3 fragPos = texture(positionBuffer, texCoord).rgb;
    vec3 albedoColor = texture(albedoBuffer, texCoord).rgb;
    float shininess = texture(albedoBuffer, texCoord).a;
	vec3 aNormal = texture(normalBuffer, texCoord).rgb;

    viewDir = normalize(viewPos - fragPos);


    //Combine lights
    vec3 result = CalcDirLight(dirLight, aNormal, texCoord, shininess, fragPos, albedoColor);
    
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], aNormal, texCoord, shininess, fragPos, albedoColor);
    
    result += CalcSpotLight(spotLight, aNormal, texCoord, shininess, fragPos, albedoColor);
    
    FragColor = vec4(result, 1.f);

}

float spec(vec3 lightDir, vec3 aNormal){
    vec3 halfwayDir = normalize(lightDir + viewDir);

    return pow(max(dot(aNormal, halfwayDir), 0.0), material.shininess);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec2 texCoord, float shininess, vec3 fragPos, vec3 albedo){
    //Set important variables
    vec3 lightDir = normalize(-light.direction);

    vec3 ambient, diffuse, specular;
    ambient = diffuse = albedo;
    specular = vec3(shininess);

    //Ambient lighting
    ambient *= light.ambient;

    //Diffuse lighting
    float diff = max(dot(normal, lightDir), 0.0);
    diffuse *= light.diffuse * diff;

    //Specular lighting
    specular *= light.specular * spec(lightDir, normal);

    return max((ambient + diffuse + specular), vec3(0.f)); //Clamp it in case it's an empty light struct
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec2 texCoord, float shininess, vec3 fragPos, vec3 albedo){
    //Set important variables
    vec3 lightDir = normalize(light.position - fragPos);

    vec3 ambient, diffuse, specular;
    ambient = diffuse = albedo;
    specular = vec3(shininess);

    //Ambient lighting
    ambient *= light.ambient;

    //Diffuse lighting
    float diff = max(dot(lightDir, normal), 0.0);
    diffuse *= light.diffuse * diff;

    //Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    //Specular lighting
    specular *= light.specular * spec(lightDir, normal);

    return max((ambient + diffuse + specular) * attenuation, vec3(0.f)); //Clamp it in case it's an empty light struct
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec2 texCoord, float shininess, vec3 fragPos, vec3 albedo){
    //Set important variables
    vec3 lightDir = normalize(light.position - fragPos);;

    vec3 ambient, diffuse, specular;
    ambient = diffuse = albedo;
    specular = vec3(shininess);

    //Ambient lighting
    ambient *= light.ambient;

    //Diffuse lighting
    float diff = max(dot(normal, lightDir), 0.0);
    diffuse *= light.diffuse * diff;

    //Specular lighting
    specular *= light.specular * spec(lightDir, normal);

    //Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    
    //Spotlight range
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    return max((ambient + diffuse + specular) * attenuation * intensity, vec3(0.f)); //Clamp it in case it's an empty light struct
}
*/
#version 330 core
out vec4 FragColor;


in vec3 normal;
in vec3 fragPos;
in vec2 texCoord;
struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_normal1;
    sampler2D texture_displacement1;

    float shininess;
};

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
struct SpotLight {
    vec3 position;
    vec3 direction;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float cutOff;
    float outerCutOff;
};
#define NR_POINT_LIGHTS 1

uniform sampler2D positionBuffer;
uniform sampler2D normalBuffer;
uniform sampler2D albedoBuffer;

uniform vec3 viewPos;

uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;

uniform Material material;

uniform bool dirLightEnabled;
uniform bool pointLightEnabled;
uniform bool spotLightEnabled;

uniform int deferredState;

//Global variables
vec3 viewDir;

//Methods
vec3 lightDir(vec3 lightPos);
float spec(vec3 lightDir, vec3 normal);
vec3 CalcDirLight(DirLight light, vec3 normal, vec2 texCoord, float shininess, vec3 fragPos, vec3 albedo);
vec3 CalcPointLight(PointLight light, vec3 normal, vec2 texCoord, float shininess, vec3 fragPos, vec3 albedo);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec2 texCoord, float shininess, vec3 fragPos, vec3 albedo);

void main(){
    //Set global variables
	vec3 fragPos = texture(positionBuffer, texCoord).rgb;
    vec3 albedoColor = texture(albedoBuffer, texCoord).rgb;
    float shininess = texture(albedoBuffer, texCoord).a;
	vec3 aNormal = texture(normalBuffer, texCoord).rgb;

    viewDir = normalize(viewPos - fragPos);
    
    //Combine lights
    vec3 result = vec3(0.f);

    if(dirLightEnabled) result += CalcDirLight(dirLight, aNormal, texCoord, shininess, fragPos, albedoColor);

    if(pointLightEnabled){
        for(int i = 0; i < NR_POINT_LIGHTS; i++)
            result += CalcPointLight(pointLights[i], aNormal, texCoord, shininess, fragPos, albedoColor);
    }
    
    if(spotLightEnabled) result += CalcSpotLight(spotLight, aNormal, texCoord, shininess, fragPos, albedoColor);
    
    if(deferredState == 0) FragColor = vec4(fragPos, 1.f);
    else if(deferredState == 1) FragColor = vec4(aNormal, 1.f);
    else if(deferredState == 2) FragColor = vec4(albedoColor, 1.f);
    else if(deferredState == 3) FragColor = vec4(vec3(shininess), 1.f);
    else if(deferredState == 4) FragColor = vec4(result, 1.f);
}

vec3 lightDir(vec3 lightPos){ return normalize(lightPos - fragPos); }
float spec(vec3 lightDir, vec3 aNormal){
    vec3 halfwayDir = normalize(lightDir + viewDir);

    return pow(max(dot(aNormal, halfwayDir), 0.0), material.shininess);
}
vec3 CalcDirLight(DirLight light, vec3 normal, vec2 texCoord, float shininess, vec3 fragPos, vec3 albedo){
    //Set important variables
    vec3 lightDir = normalize(-light.direction);

    vec3 ambient, diffuse, specular;
    ambient = diffuse = albedo;
    specular = vec3(shininess);

    //Ambient lighting
    ambient *= light.ambient;

    //Diffuse lighting
    float diff = max(dot(normal, lightDir), 0.0);
    diffuse *= light.diffuse * diff;

    //Specular lighting
    specular *= light.specular * spec(lightDir, normal);

    return max((ambient + diffuse + specular), vec3(0.f)); //Clamp it in case it's an empty light struct
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec2 texCoord, float shininess, vec3 fragPos, vec3 albedo){
    //Set important variables
    vec3 lightDir = normalize(light.position - fragPos);

    vec3 ambient, diffuse, specular;
    ambient = diffuse = albedo;
    specular = vec3(shininess);

    //Ambient lighting
    ambient *= light.ambient;

    //Diffuse lighting
    float diff = max(dot(lightDir, normal), 0.0);
    diffuse *= light.diffuse * diff;

    //Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    //Specular lighting
    specular *= light.specular * spec(lightDir, normal);

    return max((ambient + diffuse + specular) * attenuation, vec3(0.f)); //Clamp it in case it's an empty light struct
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec2 texCoord, float shininess, vec3 fragPos, vec3 albedo){
    //Set important variables
    vec3 lightDir = normalize(light.position - fragPos);;

    vec3 ambient, diffuse, specular;
    ambient = diffuse = albedo;
    specular = vec3(shininess);

    //Ambient lighting
    ambient *= light.ambient;

    //Diffuse lighting
    float diff = max(dot(normal, lightDir), 0.0);
    diffuse *= light.diffuse * diff;

    //Specular lighting
    specular *= light.specular * spec(lightDir, normal);

    //Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    
    //Spotlight range
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    return max((ambient + diffuse + specular) * attenuation * intensity, vec3(0.f)); //Clamp it in case it's an empty light struct
}