#version 420 core
out vec4 FragColor;

uniform vec3 diffuse;

void main(){
	FragColor = vec4(diffuse, 1.f);
}