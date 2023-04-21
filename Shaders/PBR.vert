#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;

out vec2 texCoord;

out vec3 someNormal;
out vec3 fragPos; 

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform vec3 pos;

uniform bool renderQuad;

out mat3 TBN;

void main(){
	fragPos = vec3(model * vec4(aPos, 1.f)) + pos;
	gl_Position = renderQuad ? vec4(aPos, 1.f) : proj * view * vec4(fragPos, 1.f); //Todo multiply it in the cpu as the cpu can save some processing
	texCoord = aTexCoord;
	someNormal = normalize(mat3(transpose(inverse(model))) * aNormal);

	
	vec3 T = normalize(vec3(model * vec4(aTangent,   0.0)));
	//vec3 B = normalize(vec3(model * vec4(aBitangent, 0.0)));
	vec3 N = normalize(vec3(model * vec4(aNormal,    0.0)));
	
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);
	
	TBN = transpose(mat3(T, B, N));
}
