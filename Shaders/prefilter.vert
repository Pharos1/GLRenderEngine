layout (location = 0) in vec3 aPos;

out vec3 localPos;

uniform mat4 proj;
uniform mat4 view;

void main(){
	localPos = aPos;
	gl_Position = proj * view* vec4(localPos, 1.f);
}