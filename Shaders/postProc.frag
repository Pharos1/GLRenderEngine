#version 330 core
out vec4 FragColor;
  
in vec2 texCoord;

const uint hdr    = 0x00000001u; //Make thoose preprocessor
const uint gammaCorrection = 0x00000002u;
const uint bloom  = 0x00000004u;
const uint blur   = 0x00000008u;
const uint other   = 0x00000032u;

uniform sampler2D colorBuffer;
uniform sampler2D bloomBlur;

uniform float exposure;
uniform float gamma;
uniform uint flags;

uniform int tonemapMode;

uniform float maxRadiance;

float luminance(vec3 v);
vec3 changeLuminance(vec3 c_in, float l_out);
vec3 reinhard(vec3 color);
vec3 reinhardExtended(vec3 color);
vec3 uncharted2Equation(vec3 x);
vec3 uncharted2(vec3 color);
vec3 manualExposure(vec3 color);
vec3 narkowiczACES(vec3 color);

//All credits Stephen Hill (@self_shadow) for writing the ACES Tone Mapping
mat3 ACESInputMat = { //The matrix is a transposed version of the original so it matches the column major order of glsl
	vec3(0.59719f, 0.07600f, 0.02840f),
	vec3(0.35458f, 0.90834f, 0.13383f),
	vec3(0.04823f, 0.01566f, 0.83777f),
};		 
mat3 ACESOutputMat = { //The matrix is a transposed version of the original so it matches the column major order of glsl
	vec3( 1.60475f, -0.10208f, -0.00327f),
	vec3(-0.53108f,  1.10813f, -0.07276f),
	vec3(-0.07367f, -0.00605f,  1.07602f),
};
vec3 RRTAndODTFit(vec3 v){
    vec3 a = v * (v + 0.0245786f) - 0.000090537f;
    vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}
vec3 hillACES(vec3 color){
    color = ACESInputMat * color;

    // Apply RRT and ODT
    color = RRTAndODTFit(color);

    color = ACESOutputMat * color;
    return color;
}

void main(){
	vec3 color = texture(colorBuffer, texCoord).rgb;
	vec3 result = vec3(0.f);
	
	//if(bool(flags & reinhard)){
	//	vec3 hdrColor = result;
  	//
	//	// exposure tone mapping
	//	//vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
	//	
	//	result = result / (1+result);
	//}
	switch(tonemapMode){
		case 1:
			result = reinhard(color);
			break;
		case 2:
			result = reinhardExtended(color);
			break;
		case 3:
			result = uncharted2(color);
			break;
		case 4:
			result = hillACES(color);
			break;
		case 5:
			result = narkowiczACES(color);
			break;
		case 6:
			result = manualExposure(color);
			break;
		default:
			result = color;
			break;
	}
	if(bool(flags & bloom)){
		result += texture(bloomBlur, texCoord).rgb;
	}
	if(bool(flags & gammaCorrection)){
		result = pow(result, vec3(1.0 / gamma));
	}
	FragColor = vec4(result, 1.f);
}
float luminance(vec3 v){ //This and changeLuminance is working only for reinhard
    return dot(v, vec3(0.2126f, 0.7152f, 0.0722f));
}
vec3 changeLuminance(vec3 c_in, float l_out){
    float l_in = luminance(c_in);
    return c_in * (l_out / l_in);
}
vec3 reinhard(vec3 color){
	float lumOld = luminance(color);
	float lumNew = lumOld / (1 + lumOld);

	return changeLuminance(color, lumNew);
}
vec3 reinhardExtended(vec3 color){
	float lumOld = luminance(color);

	float divident = lumOld * (1 + lumOld / pow(maxRadiance, 2));
	float lumNew = divident / (1 + lumOld);

	return changeLuminance(color, lumNew);
}

#define A 0.15
#define B 0.50
#define C 0.10
#define D 0.20
#define E 0.02
#define F 0.30

//Uncharted2 tonemapping is, as you might have guessed, from Uncharted2
vec3 uncharted2Equation(vec3 x){
    return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}
vec3 uncharted2(vec3 color){
	float exposure_bias = 2.0f;
	vec3 curr = uncharted2Equation(color * exposure_bias);
	
	vec3 W = vec3(11.2f);
	vec3 white_scale = vec3(1.0f) / uncharted2Equation(W);
	return curr * white_scale;
}
vec3 manualExposure(vec3 color){
    // exposure tone mapping
    vec3 result = vec3(1.0) - exp(-color * exposure);
  
    return vec4(result, 1.0);
}
#define a 2.51f
#define b 0.03f
#define c 2.43f
#define d 0.59f
#define e 0.14f

//Credit to Krzysztof Narkowicz(https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/) for his ACES approximation
vec3 narkowiczACES(vec3 color){
	return (color*(a*color+b))/(color*(c*color+d)+e);
}