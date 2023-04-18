#version 330 core
out vec4 FragColor;
  
in vec2 texCoord;

const uint hdr    = 0x00000001u;
const uint gammaCorrection = 0x00000002u;
const uint bloom  = 0x00000004u;
const uint blur   = 0x00000008u;

uniform sampler2D colorBuffer;
uniform sampler2D bloomBlur;

uniform float exposure;
uniform float gamma;
uniform uint flags;

void main(){
	vec3 result = texture(colorBuffer, texCoord).rgb;
	
	if(bool(flags & hdr)){
		vec3 hdrColor = result;
  
		// exposure tone mapping
		vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
		// gamma correction 
		result = mapped;
	}
	if(bool(flags & bloom)){
		result += texture(bloomBlur, texCoord).rgb;
	}
	if(bool(flags & gammaCorrection)){
		result = pow(result, vec3(1.0 / gamma));
	}
	FragColor = vec4(result, 1.f);
}  