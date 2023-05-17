#version 330 core
layout (location = 0) in vec3 aPos;

uniform vec3 pos;
uniform mat4 PVMat;

void main(){
	gl_Position = PVMat * vec4((aPos * 0.2) + pos, 1.f);
}