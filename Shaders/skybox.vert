#version 420 core
layout (location = 0) in vec3 aPos;

out vec3 worldPos;

uniform mat4 proj;
uniform mat4 view;

void main(){
    worldPos = aPos;

    mat4 rotView = mat4(mat3(view)); // remove translation from the view matrix
    vec4 clipPos = proj * rotView * vec4(worldPos, 1.0);

    gl_Position = clipPos.xyww;
}  