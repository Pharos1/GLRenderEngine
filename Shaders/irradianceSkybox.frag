#version 330 core
out vec4 FragColor;

in vec3 worldPos;

uniform samplerCube skybox;

void main(){
    vec3 envColor = texture(skybox, worldPos).rgb;

    envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0/2.2));
    
    //envColor = textureLod(skybox, worldPos, 1.2).rgb; //Display mipmap at specified level

    FragColor = vec4(envColor, 1.f);
}