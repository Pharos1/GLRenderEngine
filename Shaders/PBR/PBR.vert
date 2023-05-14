#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;

out vec2 texCoord;

out vec3 normal;
out vec3 worldPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform bool deferredEnabled;

out mat3 TBN;

void main(){
	worldPos = vec3(model * vec4(aPos, 1.f));
	gl_Position = deferredEnabled ? vec4(aPos, 1.f) : proj * view * vec4(worldPos, 1.f); //Todo multiply it in the cpu as the cpu can save some processing
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
	
}
