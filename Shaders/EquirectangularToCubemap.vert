#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 WorldPos;

uniform mat4 proj;
uniform mat4 view;

void main()
{
    WorldPos = aPos;
    gl_Position =  proj * view * vec4(WorldPos, 1.0);
}