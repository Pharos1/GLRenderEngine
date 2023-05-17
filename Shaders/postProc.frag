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

uniform bool fxaaEnabled;
uniform vec2 inverseScreenSize;

float luminance(vec3 v);
vec3 changeLuminance(vec3 c_in, float l_out);
vec3 reinhard(vec3 color);
vec3 reinhardExtended(vec3 color);
vec3 uncharted2Equation(vec3 x);
vec3 uncharted2(vec3 color);
vec3 manualExposure(vec3 color);
vec3 narkowiczACES(vec3 color);
vec3 fxaa();

//All credits Stephen Hill (@self_shadow) for creating the ACES Tone Mapping
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
	vec3 result = color;
	
	if(fxaaEnabled) result = fxaa();
	switch(tonemapMode){
		case 1:
			result = reinhard(result);
			break;
		case 2:
			result = reinhardExtended(result);
			break;
		case 3:
			result = uncharted2(result);
			break;
		case 4:
			result = hillACES(result);
			break;
		case 5:
			result = narkowiczACES(result);
			break;
		case 6:
			result = manualExposure(result);
			break;
		default:
			result = result;
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
float luminance(vec3 v){
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

float fxaaLuma(vec3 color) {
	//return color.g * (0.587/0.299) + color.r; //More optimized
	return sqrt(dot(color, vec3(0.299, 0.587, 0.114)));
}

//FXAA options
uniform float EDGE_THRESHOLD_MIN;
uniform float EDGE_THRESHOLD_MAX;
uniform int   ITERATIONS;
uniform float SUBPIXEL_QUALITY;

float QUALITY[7] = {1.5, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0};

/*All credits to Simon Rodriguez for explaining FXAA in detail while making it easy to comprehand(http://blog.simonrodriguez.fr/articles/2016/07/implementing_fxaa.html).*/
vec3 fxaa(){
	vec3 colorCenter = texture(colorBuffer, texCoord).rgb;

	float lumaCenter = fxaaLuma(colorCenter);
	float lumaD = fxaaLuma(textureOffset(colorBuffer, texCoord, ivec2( 0,-1)).rgb);
	float lumaU = fxaaLuma(textureOffset(colorBuffer, texCoord, ivec2( 0, 1)).rgb);
	float lumaL = fxaaLuma(textureOffset(colorBuffer, texCoord, ivec2(-1, 0)).rgb);
	float lumaR = fxaaLuma(textureOffset(colorBuffer, texCoord, ivec2( 1, 0)).rgb);
	
	float lumaMin = min(lumaCenter, min(min(lumaD, lumaU), min(lumaL,lumaR)));
	float lumaMax = max(lumaCenter, max(max(lumaD, lumaU), max(lumaL,lumaR)));
	
	float lumaRange = lumaMax - lumaMin;
	
	//If the luma variation is lower that a threshold don't perform any anti-aliasing.
	if(lumaRange < max(EDGE_THRESHOLD_MIN, lumaMax * EDGE_THRESHOLD_MAX))
		return colorCenter;

	float lumaDL = fxaaLuma(textureOffset(colorBuffer,texCoord,ivec2(-1,-1)).rgb);
	float lumaUR = fxaaLuma(textureOffset(colorBuffer,texCoord,ivec2(1,1)).rgb);
	float lumaUL = fxaaLuma(textureOffset(colorBuffer,texCoord,ivec2(-1,1)).rgb);
	float lumaDR = fxaaLuma(textureOffset(colorBuffer,texCoord,ivec2(1,-1)).rgb);
	
	float lumaDU = lumaD + lumaU;
	float lumaLR = lumaL + lumaR;
	
	float lumaLeftCorners  = lumaDL + lumaUL;
	float lumaDownCorners  = lumaDL + lumaDR;
	float lumaRightCorners = lumaDR + lumaUR;
	float lumaUpCorners    = lumaUR + lumaUL;
	
	//Estimation of the gradient along the horizontal and vertical axis.
	float edgeHorizontal =  abs(-2.0 * lumaL + lumaLeftCorners) + abs(-2.0 * lumaCenter + lumaDU) * 2.0 + abs(-2.0 * lumaR + lumaRightCorners);
	float edgeVertical   =  abs(-2.0 * lumaU + lumaUpCorners)   + abs(-2.0 * lumaCenter + lumaLR) * 2.0 + abs(-2.0 * lumaD + lumaDownCorners);
	
	bool isHorizontal = (edgeHorizontal >= edgeVertical);

	//Select the two neighboring texels lumas in the opposite direction to the local edge.
	float luma1 = isHorizontal ? lumaD : lumaL;
	float luma2 = isHorizontal ? lumaU : lumaR;

	//Compute gradients in this direction.
	float gradient1 = luma1 - lumaCenter;
	float gradient2 = luma2 - lumaCenter;
	
	bool is1Steepest = abs(gradient1) >= abs(gradient2);
	
	//Gradient in the corresponding direction
	float gradientScaled = 0.25*max(abs(gradient1),abs(gradient2));

	float stepLength = isHorizontal ? inverseScreenSize.y : inverseScreenSize.x;
	
	float lumaLocalAverage = 0.0;
	if(is1Steepest){
		//Switch the direction
		stepLength = - stepLength;
		lumaLocalAverage = 0.5*(luma1 + lumaCenter);
	} 
	else
		lumaLocalAverage = 0.5*(luma2 + lumaCenter);
	
	//Shift UV in the correct direction by half a pixel.
	vec2 currentUv = texCoord;
	if(isHorizontal)
		currentUv.y += stepLength * 0.5;
	else
		currentUv.x += stepLength * 0.5;

	//Compute offset (for each iteration step) in the right direction.
	vec2 offset = isHorizontal ? vec2(inverseScreenSize.x,0.0) : vec2(0.0,inverseScreenSize.y);

	//Compute UVs to explore on each side of the edge, orthogonally. The QUALITY allows us to step faster.
	vec2 uv1 = currentUv - offset;
	vec2 uv2 = currentUv + offset;
	
	//Read the lumas at both current extremities of the exploration segment, and compute the delta wrt to the local average luma.
	float lumaEnd1 = fxaaLuma(texture(colorBuffer,uv1).rgb);
	float lumaEnd2 = fxaaLuma(texture(colorBuffer,uv2).rgb);
	lumaEnd1 -= lumaLocalAverage;
	lumaEnd2 -= lumaLocalAverage;
	
	//If the luma deltas at the current extremities are larger than the local gradient, we have reached the side of the edge.
	bool reached1 = abs(lumaEnd1) >= gradientScaled;
	bool reached2 = abs(lumaEnd2) >= gradientScaled;
	bool reachedBoth = reached1 && reached2;
	
	//If the side is not reached, we continue to explore in this direction.
	if(!reached1)
		uv1 -= offset;
	if(!reached2)
		uv2 += offset; 
	if(!reachedBoth){
		for(int i = 2; i < ITERATIONS; i++){
			//If needed, read luma in 1st direction, compute delta.
			if(!reached1){
				lumaEnd1 = fxaaLuma(texture(colorBuffer, uv1).rgb);
				lumaEnd1 = lumaEnd1 - lumaLocalAverage;
			}
			//If needed, read luma in opposite direction, compute delta.
			if(!reached2){
				lumaEnd2 = fxaaLuma(texture(colorBuffer, uv2).rgb);
				lumaEnd2 = lumaEnd2 - lumaLocalAverage;
			}
			//If the luma deltas at the current extremities is larger than the local gradient, we have reached the side of the edge.
			reached1 = abs(lumaEnd1) >= gradientScaled;
			reached2 = abs(lumaEnd2) >= gradientScaled;
			reachedBoth = reached1 && reached2;
	
			//If the side is not reached, we continue to explore in this direction, with a variable quality.
			if(!reached1)
				uv1 -= offset * QUALITY[i];
			if(!reached2)
				uv2 += offset * QUALITY[i];
	
			//If both sides have been reached, stop the exploration.
			if(reachedBoth) break;
		}
	}
	
	//Compute the distances to each extremity of the edge.
	float distance1 = isHorizontal ? (texCoord.x - uv1.x) : (texCoord.y - uv1.y);
	float distance2 = isHorizontal ? (uv2.x - texCoord.x) : (uv2.y - texCoord.y);
	
	//In which direction is the extremity of the edge closer
	bool isDirection1 = distance1 < distance2;
	float distanceFinal = min(distance1, distance2);
	
	float edgeThickness = (distance1 + distance2);
	
	//UV offset: read in the direction of the closest side of the edge.
	float pixelOffset = - distanceFinal / edgeThickness + 0.5;

	//Is the luma at center smaller than the local average
	bool isLumaCenterSmaller = lumaCenter < lumaLocalAverage;
	
	//If the luma at center is smaller than at its neighbour, the delta luma at each end should be positive (same variation).
	//(in the direction of the closer side of the edge.)
	bool correctVariation = ((isDirection1 ? lumaEnd1 : lumaEnd2) < 0.0) != isLumaCenterSmaller;
	float finalOffset = correctVariation ? pixelOffset : 0.0;

	//Sub-pixel shifting
	//Full weighted average of the luma over the 3x3 neighborhood.
	float lumaAverage = (1.0/12.0) * (2.0 * (lumaDU + lumaLR) + lumaLeftCorners + lumaRightCorners);

	//Ratio of the delta between the global average and the center luma, over the luma range in the 3x3 neighborhood.
	float subPixelOffset1 = clamp(abs(lumaAverage - lumaCenter)/lumaRange,0.0,1.0);
	float subPixelOffset2 = (-2.0 * subPixelOffset1 + 3.0) * subPixelOffset1 * subPixelOffset1;

	//Compute a sub-pixel offset based on this delta.
	float subPixelOffsetFinal = subPixelOffset2 * subPixelOffset2 * SUBPIXEL_QUALITY;
	
	//Pick the biggest of the two offsets.
	finalOffset = max(finalOffset, subPixelOffsetFinal);
	//Compute the final UV coordinates.
	vec2 finalUv = texCoord;
	if(isHorizontal)
		finalUv.y += finalOffset * stepLength;
	else
		finalUv.x += finalOffset * stepLength;
	
	vec3 finalColor = texture(colorBuffer,finalUv).rgb;
	return finalColor;
}