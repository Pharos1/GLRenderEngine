#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec3 worldPos;

uniform samplerCube skybox;
uniform bool bloomOn;

void main(){
    vec3 envColor = texture(skybox, worldPos).rgb;

    //envColor = envColor / (envColor + vec3(1.0));
    //envColor = pow(envColor, vec3(1.0/2.2));
    
    //envColor = textureLod(skybox, worldPos, 1.2).rgb; //Display mipmap at specified level

    FragColor = vec4(envColor, 1.f);

    //Bloom
	float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722)); //Transform into a luminance value
	if(brightness > 1.0 && bloomOn)
		BrightColor = vec4(FragColor.rgb, 1.0);
	else
		BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}