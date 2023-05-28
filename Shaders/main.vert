#version 420 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;

out vec2 texCoord;

out vec3 normal;
out vec3 worldPos; 
out vec4 fragPosLightSpace;

out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

//uniform sampler2D normalMap;
uniform mat4 lightSpaceMatrix; //SHADOWS
uniform vec3 viewPos;

uniform bool deferredEnabled;
//Todo: for some operations im not sure if they will be faster making them in the cpu instead of the gpu because of: time for transfering data CPU->GPU, speed of calculation, parallelism, etc.
//Todo: not sure if i should multiply normal by tbn or multiply the other uniform (Should research some more and check the normal mapping chapter again)
void main(){
	worldPos = vec3(model * vec4(aPos, 1.f));

	if(deferredEnabled) gl_Position = vec4(aPos, 1.f);
	else gl_Position = proj * view * vec4(worldPos, 1.f);//Todo multiply it in the cpu as the cpu can save some processing

	texCoord = aTexCoord;

	mat3 normalMatrix = transpose(inverse(mat3(model))); //Transpose is really expensive function
	normal = normalize(normalMatrix * aNormal);

	if(aTangent == vec3(0.f))
		TBN = mat3(0.f);
	else{
		vec3 T = normalize(normalMatrix * aTangent);
		
		T = normalize(T - dot(T, normal) * normal);
		vec3 B = cross(normal, T);
		
		TBN = mat3(T, B, normal);
	}
	//fragPosLightSpace = lightSpaceMatrix * vec4(worldPos, 1.0); //SHADOWS
	
}
