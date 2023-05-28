#version 420 core
out vec4 FragColor;

in vec3 worldPos;

layout(binding = 0) uniform samplerCube skybox;

void main(){
    FragColor = texture(skybox, worldPos);
}