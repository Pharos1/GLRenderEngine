#version 420 core
out vec4 FragColor;

in vec2 texCoord;

layout(binding = 0) uniform sampler2D image;

void main(){
	FragColor = texture(image, texCoord);
}
