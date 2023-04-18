#version 330 core
out vec4 FragColor;
  
in vec2 texCoord;

uniform sampler2D hdrBuffer;
uniform float exposure;

uniform bool gammaCorrection;

void main()
{             
    const float gamma = 2.2;
    vec3 hdrColor = texture(hdrBuffer, texCoord).rgb;
  
    // exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
    // gamma correction 
    if(gammaCorrection)
        mapped = pow(mapped, vec3(1.0 / gamma));
  
    FragColor = vec4(mapped, 1.0);
}  