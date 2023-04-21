#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;

out vec2 texCoord;

out vec3 normal;
out vec3 fragPos; 
out vec4 fragPosLightSpace;

out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform vec3 pos;

//uniform sampler2D normalMap;
uniform mat4 lightSpaceMatrix; //SHADOWS
uniform vec3 viewPos;

uniform bool renderQuad;
//Todo: for some operations im not sure if they will be faster making them in the cpu instead of the gpu because of: time for transfering data CPU->GPU, speed of calculation, parallelism, etc.
//Todo: not sure if i should multiply normal by tbn or multiply the other uniform (Should research some more and check the normal mapping chapter again)
void main(){
	fragPos = vec3(model * vec4(aPos, 1.f)) + pos;
	gl_Position = renderQuad ? vec4(aPos, 1.f) : proj * view * vec4(fragPos, 1.f); //Todo multiply it in the cpu as the cpu can save some processing
	texCoord = aTexCoord;
	normal = normalize(mat3(transpose(inverse(model))) * aNormal);
	
	vec3 T = normalize(vec3(model * vec4(aTangent,   0.0)));
	//vec3 B = normalize(vec3(model * vec4(aBitangent, 0.0)));
	vec3 N = normalize(vec3(model * vec4(aNormal,    0.0)));
	
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);
	
	TBN = transpose(mat3(T, B, N));   
	
	fragPosLightSpace = lightSpaceMatrix * vec4(fragPos, 1.0); //SHADOWS
	
}
