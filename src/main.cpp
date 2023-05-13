/*
	
	I want to give credit to Joey DeVries, the author of 'LearnOpenGL,' for creating such a masterpiece that helped many in their journey in computer graphics.
	
*/

#include <GLAD/gl.h>
#include <GLFW/glfw3.h>
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <iostream>

#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include "Model.h"
#include "Vertex.h"
#include "Mesh.h"
#include "Light.h"
#include "Flags.h"
#include "Material.h"
#include "Query.h"

using namespace ImGui;

//Error checking
GLenum glCheckError_(const char* file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)
void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam) {
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}

void beginPostProcess();
void endPostProcess();
void renderScene(Shader& shader, Shader& PBRShader);
void processInput(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void updateImGui();
int initLibs();
void initOGL();
void initBloom();
void initPostProc();
void initDeferredShading();
void initPBRVariables();
void updateIBL();
void initImGui();
void updateModelMatrices();
void updateModelTransforms();
void updateMaterial();
void applyMaterial();
void initModels();
void setupMainLoop();
void bindUniforms(Shader& shader);

//Window & projection
GLFWwindow* window;
unsigned int SCR_WIDTH = 1280;
unsigned int SCR_HEIGHT = 720;

float fov = 45.f;
glm::mat4 model(1.f);
glm::mat4 view(1.f);
glm::mat4 proj(1.f);

RenderQuad renderQuad;
Camera cam(glm::vec3(0.f, 1.f, 3.f), 7.5f);

//Shaders
Shader shader;
//Shader skyboxShader;
Shader deferredShader;
Shader postprocShader;
Shader blurShader;
//Shader debugQuadShader;
Shader PBRShader;
Shader equirectangularToCubemapShader;
Shader hdrSkyboxShader;
Shader irradianceShader;
Shader prefilterShader;
Shader brdfShader;

//Primitives
std::vector<Vertex> cubeVertices = { // positions          // texture Coords
		Vertex(glm::vec3(0.5f,  0.5f, -0.5f), glm::vec3(0.f, 0.f, -1.f),  glm::vec2(1.0f, 1.0f)), //BACK
		Vertex(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.f, 0.f, -1.f),  glm::vec2(1.0f, 0.0f)),
		Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.f, 0.f, -1.f),  glm::vec2(0.0f, 0.0f)),
		Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.f, 0.f, -1.f),  glm::vec2(0.0f, 0.0f)),
		Vertex(glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(0.f, 0.f, -1.f),  glm::vec2(0.0f, 1.0f)),
		Vertex(glm::vec3(0.5f,  0.5f, -0.5f), glm::vec3(0.f, 0.f, -1.f),  glm::vec2(1.0f, 1.0f)),

		Vertex(glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.f, 0.f, 1.f),  glm::vec2(0.0f, 0.0f)), //FRONT
		Vertex(glm::vec3(0.5f, -0.5f,  0.5f), glm::vec3(0.f, 0.f, 1.f),  glm::vec2(1.0f, 0.0f)),
		Vertex(glm::vec3(0.5f,  0.5f,  0.5f), glm::vec3(0.f, 0.f, 1.f),  glm::vec2(1.0f, 1.0f)),
		Vertex(glm::vec3(0.5f,  0.5f,  0.5f), glm::vec3(0.f, 0.f, 1.f),  glm::vec2(1.0f, 1.0f)),
		Vertex(glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(0.f, 0.f, 1.f),  glm::vec2(0.0f, 1.0f)),
		Vertex(glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.f, 0.f, 1.f),  glm::vec2(0.0f, 0.0f)),

		Vertex(glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(-1.f, 0.f, 0.f),  glm::vec2(1.0f, 0.0f)), //LEFT
		Vertex(glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(-1.f, 0.f, 0.f),  glm::vec2(1.0f, 1.0f)),
		Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-1.f, 0.f, 0.f),  glm::vec2(0.0f, 1.0f)),
		Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-1.f, 0.f, 0.f),  glm::vec2(0.0f, 1.0f)),
		Vertex(glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(-1.f, 0.f, 0.f),  glm::vec2(0.0f, 0.0f)),
		Vertex(glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(-1.f, 0.f, 0.f),  glm::vec2(1.0f, 0.0f)),

		Vertex(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(1.f, 0.f, 0.f),  glm::vec2(0.0f, 1.0f)), //RIGHT
		Vertex(glm::vec3(0.5f,  0.5f, -0.5f), glm::vec3(1.f, 0.f, 0.f),  glm::vec2(1.0f, 1.0f)),
		Vertex(glm::vec3(0.5f,  0.5f,  0.5f), glm::vec3(1.f, 0.f, 0.f),  glm::vec2(1.0f, 0.0f)),
		Vertex(glm::vec3(0.5f,  0.5f,  0.5f), glm::vec3(1.f, 0.f, 0.f),  glm::vec2(1.0f, 0.0f)),
		Vertex(glm::vec3(0.5f, -0.5f,  0.5f), glm::vec3(1.f, 0.f, 0.f),  glm::vec2(0.0f, 0.0f)),
		Vertex(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(1.f, 0.f, 0.f),  glm::vec2(0.0f, 1.0f)),

		Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.f, -1.f, 0.f),  glm::vec2(0.0f, 1.0f)), //BOTTOM
		Vertex(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.f, -1.f, 0.f),  glm::vec2(1.0f, 1.0f)),
		Vertex(glm::vec3(0.5f, -0.5f,  0.5f), glm::vec3(0.f, -1.f, 0.f),  glm::vec2(1.0f, 0.0f)),
		Vertex(glm::vec3(0.5f, -0.5f,  0.5f), glm::vec3(0.f, -1.f, 0.f),  glm::vec2(1.0f, 0.0f)),
		Vertex(glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.f, -1.f, 0.f),  glm::vec2(0.0f, 0.0f)),
		Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.f, -1.f, 0.f),  glm::vec2(0.0f, 1.0f)),

		Vertex(glm::vec3(0.5f,  0.5f,  0.5f), glm::vec3(0.f, 1.f, 0.f),  glm::vec2(1.0f, 0.0f)), //TOP
		Vertex(glm::vec3(0.5f,  0.5f, -0.5f), glm::vec3(0.f, 1.f, 0.f),  glm::vec2(1.0f, 1.0f)),
		Vertex(glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(0.f, 1.f, 0.f),  glm::vec2(0.0f, 1.0f)),
		Vertex(glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(0.f, 1.f, 0.f),  glm::vec2(0.0f, 1.0f)),
		Vertex(glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(0.f, 1.f, 0.f),  glm::vec2(0.0f, 0.0f)),
		Vertex(glm::vec3(0.5f,  0.5f,  0.5f), glm::vec3(0.f, 1.f, 0.f),  glm::vec2(1.0f, 0.0f)),
};
std::vector<Vertex> planeVertices = {
	Vertex(glm::vec3(-1.f, 0.f, -1.f), glm::vec3(0.f, 1.f, 0.f), glm::vec2(0.f, 0.f)),
	Vertex(glm::vec3(1.f, 0.f,  1.f), glm::vec3(0.f, 1.f, 0.f), glm::vec2(1.f, 1.f)),
	Vertex(glm::vec3(1.f, 0.f, -1.f), glm::vec3(0.f, 1.f, 0.f), glm::vec2(1.f, 0.f)),

	Vertex(glm::vec3(-1.f, 0.f, -1.f), glm::vec3(0.f, 1.f, 0.f), glm::vec2(0.f, 0.f)),
	Vertex(glm::vec3(-1.f, 0.f,  1.f), glm::vec3(0.f, 1.f, 0.f), glm::vec2(0.f, 1.f)),
	Vertex(glm::vec3(1.f, 0.f,  1.f), glm::vec3(0.f, 1.f, 0.f), glm::vec2(1.f, 1.f)),

};
std::vector<Vertex> quadVertices = {
	Vertex(glm::vec3(-1.f,-1.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec2(0.f, 0.f)),
	Vertex(glm::vec3(1.f,-1.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec2(1.f, 0.f)),
	Vertex(glm::vec3(1.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec2(1.f, 1.f)),

	Vertex(glm::vec3(-1.f,-1.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec2(0.f, 0.f)),
	Vertex(glm::vec3(1.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec2(1.f, 1.f)),
	Vertex(glm::vec3(-1.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec2(0.f, 1.f)),
};

//Post Processing
unsigned int postprocFBO, postprocColorBuffer, postprocRBO;
PostProcFlags postprocFlags(PostProcFlag_gammaCorrection);

float exposure = 2.2f;
float gamma = 2.2f;

unsigned int bloomFBO;
unsigned int colorBuffers[2];

unsigned int pingpongFBO[2];
unsigned int pingpongBuffers[2];

//GL OPTIONS
bool MSAA = true;
bool depthTest = true;
bool cullFace = true;
bool glGamma = true;
glm::vec3 clearColor(0.f, 0.f, 0.f);

//Lights
bool dirLightEnabled = true;
bool pointLightEnabled = false;
bool spotLightEnabled = false;

DirLight dirLight(glm::vec3(-1.f, -1.f, -1.f), glm::vec3(1.f));//(glm::vec3(-1.f, -1.f, -1.f), glm::vec3(.1f), glm::vec3(1.f), glm::vec3(.3f));
PointLight pointLight(glm::vec3(0.f, .5f, 3.f), glm::vec3(1.f), 1.f, .014f, .000007f);//(glm::vec3(0.f, .5f, 3.f), glm::vec3(.05f), glm::vec3(1.f), glm::vec3(.3f), 1.f, .014f, .000007f);
SpotLight spotLight(cam.getPos(), cam.camFront, glm::vec3(1.f), 1.f, .09f, .032f, 12.5f, 15.f);//(cam.getPos(), cam.camFront, glm::vec3(0.f), glm::vec3(1.f), glm::vec3(.3f), 1.f, .09f, .032f, 12.5f, 15.f);

//Deferred Shading
bool deferredShadingEnabled = false;
int deferredState = 4;

//Deferred shading
unsigned int gBuffer;
unsigned int gPosition, gNormal, gAlbedoSpec;

//PBR & IBL
HDRMap hdrTexture;
ClassicMesh PBRSkybox;
GLuint envCubemap;

GLuint captureFBO;
GLuint captureRBO;

GLuint irradianceMap;
GLuint prefilterMap;
GLuint brdfLUTTexture;

glm::mat4 captureViews[] ={
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
};

//IMGUI
ImGuiIO io;
ImGuiStyle* style;
float windowAlpha;

//GUI options
bool pbrEnabled = true;
bool iblSpecularity = true;
int materialState = 5;
bool demoRotation = false;
bool transformSRGB = true;
bool useAlbedo = true;
bool useNormalMap = true;
bool useMetallic = true;
bool useRoughness = true;
bool useAmbientMap = true;
int currentSkybox = 2;
int currentModel = 0;
int tonemapMode = 5;
float maxRadiance = 1.f;

//Scene
Model gun;
Model suzanne;
Model backpack;
Model* modelPtr;

Material material;

glm::vec3 modelPos(0.f);
glm::vec3 modelScale(1.f);
glm::vec3 modelRot(0.f);
float modelRotAngle = 0.f;
glm::mat4 modelMat(1.f);
glm::mat4 rotationMat(1.f);
glm::mat4 modelDemoRot(1.f);

//Query
Query guiPass;
Query renderPass;
Query postprocPass;

//Others
bool mouseLocked = true;

int main() {
	if(initLibs()) return -1; //If result is different than 0 then stop
	initOGL();
	
	//Shaders
	shader = Shader("Shaders/main.vert", "Shaders/main.frag");
	//skyboxShader = Shader("Shaders/skybox.vert", "Shaders/skybox.frag");
	deferredShader = Shader("Shaders/main.vert", "Shaders/deferred.frag");
	postprocShader = Shader("Shaders/renderQuad.vert", "Shaders/postProc.frag");
	blurShader = Shader("Shaders/renderQuad.vert", "Shaders/blur.frag");
	//debugQuadShader = Shader("Shaders/renderQuad.vert", "Shaders/renderQuad.frag");
	PBRShader = Shader("Shaders/PBR/PBR.vert", "Shaders/PBR/PBR.frag");
	equirectangularToCubemapShader = Shader("Shaders/cubemap.vert", "Shaders/PBR/EquirectangularToCubemap.frag");
	hdrSkyboxShader = Shader("Shaders/skybox.vert", "Shaders/PBR/hdrSkybox.frag");
	irradianceShader = Shader("Shaders/cubemap.vert", "Shaders/PBR/irradianceConvolution.frag");
	prefilterShader = Shader("Shaders/cubemap.vert", "Shaders/PBR/prefilter.frag");
	brdfShader = Shader("Shaders/PBR/brdfShader.vert", "Shaders/PBR/brdfShader.frag");

	//debugQuadShader.use();
	//debugQuadShader.set1i("image", 0);

	shader.use();

	//Matrices
	proj = glm::perspective(glm::radians(fov), SCR_WIDTH / (float)SCR_HEIGHT, .1f, 100.f);

	shader.setMat4("model", model);
	shader.setMat4("view", view);
	shader.setMat4("proj", proj);

	renderQuad = RenderQuad(quadVertices);

	//Skybox
	std::vector<std::string> textureFaces = {
		"Images/skybox/right.jpg",
		"Images/skybox/left.jpg",
		"Images/skybox/top.jpg",
		"Images/skybox/bottom.jpg",
		"Images/skybox/front.jpg",
		"Images/skybox/back.jpg"
	};
	//skybox = Skybox(textureFaces);

	//Lights
	dirLight.set(shader, "dirLight");
	pointLight.set(shader, "pointLights[0]");
	spotLight.set(shader, "spotLight");

	shader.set1b("dirLightEnabled", dirLightEnabled);
	shader.set1b("pointLightEnabled", pointLightEnabled);
	shader.set1b("spotLightEnabled", spotLightEnabled);

	/* SHADOWS //NOTE: I can use this code for dirLight. For spotLight use this but with perspective, and for pointLight use the cubemap shadow version(with perspective projection).
	glm::vec3 lightPos = glm::vec3(-3.0f, 6.0f, -2.0f);

	shader.setVec3("dirLight.direction", -lightPos);
	shader.setVec3("dirLight.ambient", glm::vec3(.1f));
	shader.setVec3("dirLight.diffuse", glm::vec3(1.f));
	shader.setVec3("dirLight.specular", glm::vec3(.3f));

	ClassicMesh shadowGround(planeVertices, {}, { Texture("Images/wood.png"), Texture("Images/White.png", "texture_specular")});
	ClassicMesh shadowCube(cubeVertices, {}, { Texture("Images/container2.png"), Texture("Images/container2_specular.png", "texture_specular") });

	glm::mat4 shadowGroundModel = planeModel;
	glm::mat4 shadowCube1Model(1.f);
	glm::mat4 shadowCube2Model(1.f);
	glm::mat4 shadowCube3Model(1.f);

	shadowCube1Model = glm::translate(shadowCube1Model, glm::vec3(2.f, 0.5001f, 3.f));
	shadowCube2Model = glm::translate(shadowCube2Model, glm::vec3(-2.f, 3.f, 3.f));
	shadowCube3Model = glm::translate(shadowCube3Model, glm::vec3(2.f, 2.f, -1.f));
	shadowCube3Model = glm::rotate(shadowCube3Model, glm::radians(45.f), glm::vec3(1.f, 1.f, 0.f));

	//Shadow Map
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

	// create depth texture
	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	float near_plane = 1.0f, far_plane = 15.f;
	glm::mat4 lightProj = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);

	glm::mat4 lightView = glm::lookAt(lightPos,
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 lightSpaceMatrix = lightProj * lightView;

	Shader simpleDepthShader("Shaders/shadow.vert", "Shaders/shadow.frag");
	*/
	/*SHADOWS(cubemap)
	ClassicMesh shadowGround(planeVertices, {}, { ClassicTexture("Images/wood.png"), ClassicTexture("Images/White.png", "texture_specular") });
	ClassicMesh shadowCube(cubeVertices, {}, { ClassicTexture("Images/container2.png"), ClassicTexture("Images/container2_specular.png", "texture_specular") });

	glm::mat4 shadowGroundModel = planeModel;
	glm::mat4 shadowCube1Model(1.f);
	glm::mat4 shadowCube2Model(1.f);
	glm::mat4 shadowCube3Model(1.f);

	shadowCube1Model = glm::translate(shadowCube1Model, glm::vec3(2.f, 0.5001f, 3.f));
	shadowCube2Model = glm::translate(shadowCube2Model, glm::vec3(-2.f, 3.f, 3.f));
	shadowCube3Model = glm::translate(shadowCube3Model, glm::vec3(2.f, 2.f, -1.f));
	shadowCube3Model = glm::rotate(shadowCube3Model, glm::radians(45.f), glm::vec3(1.f, 1.f, 0.f));

	glm::vec3 lightPos = pointLight.pos;

	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	unsigned int depthCubemap;
	glGenTextures(1, &depthCubemap);

	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	for (unsigned int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
			SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;
	float near = 1.0f;
	float far = 25.0f;
	glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, near, far);

	std::vector<glm::mat4> shadowTransforms;
	shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0,-1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0, 0.0,-1.0), glm::vec3(0.0, -1.0, 0.0)));

	Shader simpleDepthShader("Shaders/cubemapShadow.vert", "Shaders/cubemapShadow.frag", "Shaders/cubemapShadow.geom");
	*/

	initBloom();
	initDeferredShading();
	initPostProc();

	//PBR
	Material::setIndices(PBRShader);
	Material::setIndices(shader);

	initPBRVariables();
	updateIBL();

	initModels();
	updateModelTransforms();

	updateMaterial();
	applyMaterial();

	//IMGUI
	initImGui();

	//Queries
	guiPass = Query(GL_TIME_ELAPSED);
	renderPass = Query(GL_TIME_ELAPSED);
	postprocPass = Query(GL_TIME_ELAPSED);

	while (!glfwWindowShouldClose(window)) {
		//glCheckError();
		setupMainLoop();

		/* SHADOWS
		//render scene from light's point of view
		simpleDepthShader.use();
		simpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);

		//glCullFace(GL_FRONT);

		simpleDepthShader.setMat4("model", shadowGroundModel);
		shadowGround.Draw(simpleDepthShader);

		simpleDepthShader.setMat4("model", shadowCube1Model);
		shadowCube.Draw(simpleDepthShader);
		simpleDepthShader.setMat4("model", shadowCube2Model);
		shadowCube.Draw(simpleDepthShader);
		simpleDepthShader.setMat4("model", shadowCube3Model);
		shadowCube.Draw(simpleDepthShader);

		//glCullFace(GL_BACK);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// reset viewport
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();
		shader.set1i("shadowMap", 2);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

		shader.setMat4("model", shadowGroundModel);
		shadowGround.Draw(shader);

		shader.setMat4("model", shadowCube1Model);
		shadowCube.Draw(shader);
		shader.setMat4("model", shadowCube2Model);
		shadowCube.Draw(shader);
		shader.setMat4("model", shadowCube3Model);
		shadowCube.Draw(shader);
		*/
		/* SHADOWS(cubemap)
		//render scene from light's point of view
		simpleDepthShader.use();

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);

		simpleDepthShader.setVec3("lightPos", pointLight.pos);
		simpleDepthShader.set1f("far_plane", far);

		for (int i = 0; i < shadowTransforms.size(); i++) {
			simpleDepthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
		}

		renderScene(simpleDepthShader);

		//simpleDepthShader.setMat4("model", shadowGroundModel);
		//shadowGround.Draw(simpleDepthShader);
		//
		//simpleDepthShader.setMat4("model", shadowCube1Model);
		//shadowCube.Draw(simpleDepthShader);
		//simpleDepthShader.setMat4("model", shadowCube2Model);
		//shadowCube.Draw(simpleDepthShader);
		//simpleDepthShader.setMat4("model", shadowCube3Model);
		//shadowCube.Draw(simpleDepthShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// 2. then render scene as normal with shadow mapping (using depth cubemap)
		shader.use();

		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.set1i("depthMap", 2);
		shader.set1f("far_plane", far);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

		renderScene(shader);

		//shader.setMat4("model", shadowGroundModel);
		//shadowGround.Draw(shader);
		//
		//shader.setMat4("model", shadowCube1Model);
		//shadowCube.Draw(shader);
		//shader.setMat4("model", shadowCube2Model);
		//shadowCube.Draw(shader);
		//shader.setMat4("model", shadowCube3Model);
		//shadowCube.Draw(shader);
		*/

		if (deferredShadingEnabled) {
			glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			deferredShader.use();
		
			deferredShader.setMat4("proj", proj);
			deferredShader.setMat4("view", view);
		
			renderScene(deferredShader, PBRShader);
		
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
			beginPostProcess();

			shader.use();
			shader.set1b("deferredEnabled", true);
			renderQuad.Draw(shader, { gPosition, gNormal, gAlbedoSpec });
			shader.set1b("deferredEnabled", false);
		}
		else {
			beginPostProcess();
			renderScene(shader, PBRShader);
		}
		endPostProcess();

		updateImGui();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	//End of program
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}
void beginPostProcess() {
	if (postprocFlags.hasFlag(PostProcFlag_bloom))
		glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO);
	else
		glBindFramebuffer(GL_FRAMEBUFFER, postprocFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void endPostProcess() {
	postprocPass.begin();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (postprocFlags.hasFlag(PostProcFlag_bloom)) {
		bool horizontal = true, first_iteration = true;
		int amount = 10;


		blurShader.use();

		for (unsigned int i = 0; i < amount; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);

			blurShader.set1i("horizontal", horizontal);

			renderQuad.Draw(blurShader, { first_iteration ? colorBuffers[1] : pingpongBuffers[!horizontal] });

			horizontal = !horizontal;
			if (first_iteration)
				first_iteration = false;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		renderQuad.Draw(postprocShader, { colorBuffers[0], pingpongBuffers[1] });
	}
	else
		renderQuad.Draw(postprocShader, { postprocColorBuffer });

	postprocPass.end();
}
void renderScene(Shader& shader, Shader& PBRShader) {
	/*shader.use();

	shader.setMat4("model", planeModel);
	shader.setVec3("pos", planePos);
	plane.Draw(shader);
	shader.setMat4("model", glm::mat4(1.f)); //Set the uniform to default

	//shader.setMat4("model", cube1Model);
	//cube1Model = glm::rotate(cube1Model, glm::radians(0.1f), glm::vec3(0.f, 1.f, 0.f));
	shader.setVec3("pos", cube1Pos);
	shader.setMat4("model", cube1Model);
	cube.Draw(shader);

	//shader.setMat4("model", cube2Model);
	shader.setVec3("pos", cube2Pos);
	cube.Draw(shader);

	//Draw Skybox last
	if (drawSkybox) skybox.Draw(skyboxShader, view, proj);
	*/
	renderPass.begin();

	if (pbrEnabled) {
		//Assign textures
		PBRShader.use();
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);

		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);

		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);

		modelPtr->Draw(PBRShader);
	}
	else {
		shader.use();
		modelPtr->Draw(shader);
	}

	glDepthFunc(GL_LEQUAL);
	hdrSkyboxShader.use();
	hdrSkyboxShader.setMat4("view", view);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	PBRSkybox.Draw(hdrSkyboxShader);
	
	//Default the options
	glDepthFunc(GL_LESS);


	renderPass.end();
}
void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		mouseLocked = false;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		style->Alpha = windowAlpha;
	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS && !io.WantCaptureMouse) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		mouseLocked = true;
		style->Alpha = 0.3f;
	}
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	if (!glfwGetWindowAttrib(window, GLFW_FOCUSED)) //Protect from frambuffer error when alt+tabbing in fullscreen(Cuz alt+tab calls the func)
		return;
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	proj = glm::perspective(glm::radians(fov), SCR_WIDTH / (float)SCR_HEIGHT, .1f, 100.f);
	
	shader.use();
	shader.setMat4("proj", proj);
	
	PBRShader.use();
	PBRShader.setMat4("proj", proj);

	initPostProc(); //Update it's frame buffer
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (mouseLocked)
		cam.processMouse(xpos, ypos);
	else {
		cam.lastX = xpos; //DO this so the mouse dont skip when entering in view mode
		cam.lastY = ypos;
	}
}
void updateImGui() {
	guiPass.begin();

	ImGuiStyle& style = GetStyle();

	SetNextWindowSizeConstraints(ImVec2(200, SCR_HEIGHT), ImVec2(SCR_WIDTH, SCR_HEIGHT));
	SetNextWindowBgAlpha(.7f);

	Begin("Option Window", NULL, ImGuiWindowFlags_NoMove);
	SetWindowPos(ImVec2(SCR_WIDTH - GetWindowWidth(), 0));
	
	if (CollapsingHeader("Rendering", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (TreeNode("Objects")) {
			//Text("Skybox");
			//Checkbox("Draw Skybox", &drawSkybox);
			//
			//Text("Cube1");
			//Checkbox("Enabled##0", &cube1Enabled);
			//BeginDisabled(!cube1Enabled);
			//SliderFloat3("Pos##0", glm::value_ptr(cube1Pos), -10, 10);
			//EndDisabled();
			//
			//Text("Cube2");
			//Checkbox("Enabled##1", &cube2Enabled);
			//BeginDisabled(!cube2Enabled);
			//SliderFloat3("Pos##1", glm::value_ptr(cube2Pos), -10, 10);
			//EndDisabled();
			//
			//Text("Plane");
			//Checkbox("Enabled##2", &planeEnabled);
			//BeginDisabled(!planeEnabled);
			//SliderFloat3("Pos##2", glm::value_ptr(planePos), -10, 10);
			//EndDisabled();
			Checkbox("Demo Rotation", &demoRotation);

			if (SliderFloat3("Position", glm::value_ptr(modelPos), -10.f, 10.f) ||
				SliderFloat3("Scale", glm::value_ptr(modelScale), 0.f, 1.f) ||
				SliderFloat3("Rotation Axis", glm::value_ptr(modelRot), -1.f, 1.f) ||
				SliderFloat("Rotation Angle", &modelRotAngle, 0.f, 360.f)) {

				updateModelMatrices();
			}
			NewLine();

			if (TreeNode("Material")) {
				//RadioButton("Gold", &materialState, 0);
				//RadioButton("Grass", &materialState, 1);
				//RadioButton("Plastic", &materialState, 2);
				//RadioButton("Rusted Iron", &materialState, 3);
				//RadioButton("Wall", &materialState, 4);
				//RadioButton("Default Model Texture", &materialState, 5);
				const char* materials[] = { "Gold", "Grass", "Plastic", "Rusted Iron", "Wall", "Default Model Texture" };
				Combo("Material##0", &materialState, materials, 6);

				NewLine();
				if (Button("Apply", ImVec2(50.f, 20.f)))
					updateMaterial();

				TreePop();
			}
			if (TreeNode("Model")) {
				std::vector<const char*> models = { "PBR Gun", "Suzanne", "Backpack"};

				PushItemWidth(GetWindowWidth() - 130);
				Combo("Current Model", &currentModel, models.data(), models.size());
				PopItemWidth();

				NewLine();
				if (Button("Apply", ImVec2(50.f, 20.f)))
					updateModelTransforms();

				TreePop();
			}

			TreePop();
		}
		if (TreeNode("Lights")) {
			/*
			bool changed;
			Text("Directional Light");
			if (Checkbox("Enable Light##0", &dirLightEnabled)) shader.set1b("dirLightEnabled", dirLightEnabled);
			BeginDisabled(!dirLightEnabled);
			if (ColorEdit3("Ambient##0", glm::value_ptr(dirLight.ambient))) changed = true;
			if (ColorEdit3("Diffuse##0", glm::value_ptr(dirLight.diffuse))) changed = true;
			if (ColorEdit3("Specular##0", glm::value_ptr(dirLight.specular))) changed = true;
			if (SliderFloat3("Dir", glm::value_ptr(dirLight.dir), -1, 1)) changed = true;
			if (changed) {
				shader.use();
				dirLight.set(shader, "dirLight");
				changed = false;
			}
			EndDisabled();

			Text("Point Light");
			if (Checkbox("Enable Light##1", &pointLightEnabled)) shader.set1b("pointLightEnabled", pointLightEnabled);
			BeginDisabled(!pointLightEnabled);
			if (ColorEdit3("Ambient##1", glm::value_ptr(pointLight.ambient)))  changed = true;
			if (ColorEdit3("Diffuse##1", glm::value_ptr(pointLight.diffuse)))  changed = true;
			if (ColorEdit3("Specular##1", glm::value_ptr(pointLight.specular)))  changed = true;
			if (SliderFloat3("Pos##3", glm::value_ptr(pointLight.pos), -10, 10)) changed = true;
			if (changed) {
				shader.use();
				pointLight.set(shader, "pointLights[0]");
				changed = false;
			}
			EndDisabled();

			Text("Spot Light");
			if (Checkbox("Enable Light##2", &spotLightEnabled)) shader.set1b("spotLightEnabled", spotLightEnabled);
			BeginDisabled(!spotLightEnabled);
			if (ColorEdit3("Ambient##2", glm::value_ptr(spotLight.ambient)))  changed = true;
			if (ColorEdit3("Diffuse##2", glm::value_ptr(spotLight.diffuse)))  changed = true;
			if (ColorEdit3("Specular##2", glm::value_ptr(spotLight.specular)))  changed = true;
			if (changed) {
				shader.use();
				spotLight.set(shader, "spotLight");
				changed = false;
			}
			EndDisabled();
			*/
			bool nodeOpened;

			//Remove hover and active colors
			glm::vec2 oldW(style.Colors[ImGuiCol_HeaderHovered].w, style.Colors[ImGuiCol_HeaderActive].w);

			style.Colors[ImGuiCol_HeaderHovered].w = 0.0f;
			style.Colors[ImGuiCol_HeaderActive].w = 0.0f;

			//Dir Light
			nodeOpened = TreeNodeEx("Dir Light", ImGuiTreeNodeFlags_OpenOnArrow);

			SameLine();
			if (Checkbox("##0", &dirLightEnabled)) {
				shader.use();
				shader.set1b("dirLightEnabled", dirLightEnabled);
				PBRShader.use();
				PBRShader.set1b("dirLightEnabled", dirLightEnabled);
			}

			BeginDisabled(!dirLightEnabled);
			if (nodeOpened) {
				if (ColorEdit3("Diffuse##0", glm::value_ptr(dirLight.diffuse))) {
					shader.use();
					shader.setVec3("dirLight.diffuse", dirLight.diffuse);
					PBRShader.use();
					PBRShader.setVec3("dirLight.diffuse", dirLight.diffuse);
				}

				if (SliderFloat3("Dir##0", glm::value_ptr(dirLight.dir), -1.f, 1.f)){
					shader.use();
					shader.setVec3("dirLight.direction", dirLight.dir);
					PBRShader.use();
					PBRShader.setVec3("dirLight.direction", dirLight.dir);
				}
				NewLine(); //If opened make some space
				TreePop();
			}
			EndDisabled();


			//Point Light
			nodeOpened = TreeNodeEx("Point Light", ImGuiTreeNodeFlags_OpenOnArrow);
			SameLine();
			if (Checkbox("##1", &pointLightEnabled)) {
				shader.use();
				shader.set1b("pointLightEnabled", pointLightEnabled);
				PBRShader.use();
				PBRShader.set1b("pointLightEnabled", pointLightEnabled);
			}

			BeginDisabled(!pointLightEnabled);
			if (nodeOpened) {
				if (ColorEdit3("Diffuse##1", glm::value_ptr(pointLight.diffuse))){
					shader.use();
					shader.setVec3("pointLights[0].diffuse", pointLight.diffuse);
					PBRShader.use();
					PBRShader.setVec3("pointLights[0].diffuse", pointLight.diffuse);
				}

				if (SliderFloat3("Pos##1", glm::value_ptr(pointLight.pos), -10, 10)){
					shader.use();
					shader.setVec3("pointLights[0].position", pointLight.pos);

					PBRShader.use();
					PBRShader.setVec3("pointLights[0].position", pointLight.pos);
				}
				NewLine(); //If opened make some space
				TreePop();
			}
			EndDisabled();


			//Spot Light
			nodeOpened = TreeNodeEx("Spot Light", ImGuiTreeNodeFlags_OpenOnArrow);
			SameLine();
			if (Checkbox("##2", &spotLightEnabled)) {
				shader.use();
				shader.set1b("spotLightEnabled", spotLightEnabled);
				PBRShader.use();
				PBRShader.set1b("spotLightEnabled", spotLightEnabled);
			}

			BeginDisabled(!spotLightEnabled);
			if (nodeOpened) {
				if (ColorEdit3("Diffuse##2", glm::value_ptr(spotLight.diffuse))){
					shader.use();
					shader.setVec3("spotLight.diffuse", spotLight.diffuse);
					PBRShader.use();
					PBRShader.setVec3("spotLight.diffuse", spotLight.diffuse);
				}

				if (SliderFloat("Inner Cutoff", &spotLight.cutOff, 0.f, 180.f)) {
					spotLight.updateCosCutOff();
					PBRShader.set1f("spotLight.cutOff", spotLight.cosCutOff);
				}
				if (SliderFloat("Outer Cutoff", &spotLight.outerCutOff, 0.f, 180.f)) {
					spotLight.updateCosOuterCutOff();
					PBRShader.set1f("spotLight.outerCutOff", spotLight.cosOuterCutOff);
				}
				NewLine(); //If opened make some space
				TreePop();
			}
			EndDisabled();
			style.Colors[ImGuiCol_HeaderHovered].w = oldW.x;
			style.Colors[ImGuiCol_HeaderActive].w = oldW.y;

			TreePop();
		}
		if (TreeNode("Deferred Shading/Rendering")) {
			Checkbox("Enable Deferred Shading", &deferredShadingEnabled);
			BeginDisabled(!deferredShadingEnabled);
			if (RadioButton("Display Position Buffer", &deferredState, 0) ||
				RadioButton("Display Normal Buffer", &deferredState, 1)   ||
				RadioButton("Display Albedo Buffer", &deferredState, 2)   ||
				RadioButton("Display Specular Buffer", &deferredState, 3) ||
				RadioButton("Display Combined", &deferredState, 4)) {
					shader.use();
					shader.set1i("deferredState", deferredState);
			}
			EndDisabled();
			TreePop();
		}
		if (TreeNode("PBR")) {
			Checkbox("Use PBR", &pbrEnabled);
			NewLine();

			BeginDisabled(!pbrEnabled);

			PBRShader.use();

			//bool nodeOpened;
			//
			////Remove hover and active colors
			//glm::vec2 oldW(style.Colors[ImGuiCol_HeaderHovered].w, style.Colors[ImGuiCol_HeaderActive].w);
			//
			//style.Colors[ImGuiCol_HeaderHovered].w = 0.0f;
			//style.Colors[ImGuiCol_HeaderActive].w = 0.0f;
			//
			////Dir Light
			//nodeOpened = TreeNodeEx("PBR Dir Light", ImGuiTreeNodeFlags_OpenOnArrow);
			//
			//SameLine();
			//if (Checkbox("##0", &dirLightEnabled)) PBRShader.set1b("dirLightEnabled", dirLightEnabled);
			//
			//BeginDisabled(!dirLightEnabled);
			//if (nodeOpened) {
			//	if (ColorEdit3("Diffuse##3", glm::value_ptr(dirLight.diffuse)))
			//		PBRShader.setVec3("dirLight.diffuse", dirLight.diffuse);
			//
			//	if (SliderFloat3("Dir##1", glm::value_ptr(dirLight.dir), -1.f, 1.f))
			//		PBRShader.setVec3("dirLight.direction", dirLight.dir);
			//	NewLine(); //If opened make some space
			//	TreePop();
			//}
			//EndDisabled();
			//
			//
			////Point Light
			//nodeOpened = TreeNodeEx("PBR Point Light", ImGuiTreeNodeFlags_OpenOnArrow);
			//SameLine();
			//if (Checkbox("##1", &pointLightEnabled)) PBRShader.set1b("pointLightEnabled", pointLightEnabled);
			//
			//BeginDisabled(!pointLightEnabled);
			//if (nodeOpened) {
			//	if (ColorEdit3("Diffuse##4", glm::value_ptr(pointLight.diffuse)))
			//		PBRShader.setVec3("pointLights[0].diffuse", pointLight.diffuse);
			//
			//	if (SliderFloat3("Pos##4", glm::value_ptr(pointLight.pos), -10, 10))
			//		PBRShader.setVec3("pointLights[0].position", pointLight.pos);
			//	NewLine(); //If opened make some space
			//	TreePop();
			//}
			//EndDisabled();
			//
			//
			////Spot Light
			//nodeOpened = TreeNodeEx("PBR Spot Light", ImGuiTreeNodeFlags_OpenOnArrow);
			//SameLine();
			//if (Checkbox("##2", &spotLightEnabled)) PBRShader.set1b("spotLightEnabled", spotLightEnabled);
			//
			//BeginDisabled(!spotLightEnabled);
			//if (nodeOpened) {
			//	if (ColorEdit3("Diffuse##5", glm::value_ptr(spotLight.diffuse)))
			//		PBRShader.setVec3("spotLight.diffuse", spotLight.diffuse);
			//
			//	if (SliderFloat("Inner Cutoff", &spotLight.cutOff, 0.f, 180.f)) {
			//		spotLight.updateCosCutOff();
			//		PBRShader.set1f("spotLight.cutOff", spotLight.cosCutOff);
			//	}
			//	if (SliderFloat("Outer Cutoff", &spotLight.outerCutOff, 0.f, 180.f)) {
			//		spotLight.updateCosOuterCutOff();
			//		PBRShader.set1f("spotLight.outerCutOff", spotLight.cosOuterCutOff);
			//	}
			//	NewLine(); //If opened make some space
			//	TreePop();
			//}
			//EndDisabled();
			//style.Colors[ImGuiCol_HeaderHovered].w = oldW.x;
			//style.Colors[ImGuiCol_HeaderActive].w = oldW.y;


			if (Checkbox("Use IBL Specularity", &iblSpecularity))
				PBRShader.set1b("iblSpecularity", iblSpecularity);
			NewLine();

			if (Checkbox("Use Albedo Texture", &useAlbedo)) {
				PBRShader.use();
				PBRShader.set1b("useAlbedo", useAlbedo);
			}
			if (Checkbox("Use Normal Map", &useNormalMap)) {
				PBRShader.use();
				PBRShader.set1b("useNormalMap", useNormalMap);
			}
			if (Checkbox("Use Metallic Texture", &useMetallic)) {
				PBRShader.use();
				PBRShader.set1b("useMetallic", useMetallic);
			}
			if (Checkbox("Use Roughness Texture", &useRoughness)) {
				PBRShader.use();
				PBRShader.set1b("useRoughness", useRoughness);
			}
			if (Checkbox("Use Ambient Map", &useAmbientMap)) {
				PBRShader.use();
				PBRShader.set1b("useAmbientMap", useAmbientMap);
			}

			EndDisabled();
			TreePop();
		}
		if (TreeNode("Skybox")) {
			if (RadioButton("Abandoned Room 2K", &currentSkybox, 0)) {
				hdrTexture = HDRMap("Images/abandoned_tiled_room_2k.hdr");
				updateIBL();
			}
			if (RadioButton("Abandoned Room 4K", &currentSkybox, 1)) {
				hdrTexture = HDRMap("Images/abandoned_tiled_room_4k.hdr");
				updateIBL();
			}
			if (RadioButton("Grass Field", &currentSkybox, 2)) {
				hdrTexture = HDRMap("Images/HDR_029_Sky_Cloudy_Ref.hdr");
				updateIBL();
			}
			if (RadioButton("Thatch Chapel 2K", &currentSkybox, 3)) {
				hdrTexture = HDRMap("Images/thatch_chapel_2k.hdr");
				updateIBL();
			}
			if (RadioButton("Thatch Chapel 4K", &currentSkybox, 4)) {
				hdrTexture = HDRMap("Images/thatch_chapel_4k.hdr");
				updateIBL();
			}

			TreePop();
		}
	}
	if (CollapsingHeader("Post-processing", ImGuiTreeNodeFlags_DefaultOpen)) {
		//HDR, MSAA, blur, bloom, blur and such
		if (Checkbox("Using textures in sRGB color space?", &transformSRGB)) {
			PBRShader.use();
			PBRShader.set1b("transformSRGB", transformSRGB);

			shader.use();
			shader.set1b("transformSRGB", transformSRGB);
		}
		NewLine();

		if (TreeNode("Tonemapping")) {
			std::vector<const char*> tonemapModes = { "None", "Reinhard", "Extended Reinhard", "Uncharted2", "Hill ACES", "Narkowicz ACES", "Manual Exposure"};

			if (Combo("Tonemap Mode", &tonemapMode, tonemapModes.data(), tonemapModes.size())) {
				postprocShader.use();
				postprocShader.set1i("tonemapMode", tonemapMode);
			}
			NewLine();

			if (SliderFloat("Maximum Radiance", &maxRadiance, 0, 10)) {
				postprocShader.use();
				postprocShader.set1f("maxRadiance", maxRadiance);
			}
			if (SliderFloat("Manual Exposure", &exposure, 0, 10)) {
				postprocShader.use();
				postprocShader.set1f("exposure", exposure);
			}

			TreePop();
		}

		bool tempBool = postprocFlags.hasFlag(PostProcFlag_gammaCorrection);
		if (TreeNode("Gamma Correction")) {
			if (Checkbox("Use Gamma Correction", &tempBool)) {
				postprocFlags.flipFlag(PostProcFlag_gammaCorrection);
				postprocFlags.set(postprocShader, "flags");
			}

			BeginDisabled(!tempBool);
			if (SliderFloat("Gamma", &gamma, 0.f, 10.f)) {
				postprocShader.use();
				postprocShader.set1f("gamma", gamma);
			}

			EndDisabled();
			TreePop();
		}
		//NewLine();
		//
		//tempBool = postprocFlags.hasFlag(PostProcFlag_bloom);
		//if (Checkbox("Bloom |TODO|", &tempBool)) { //Todo: its working but not with tonemapping as tonemapping should be done before bloom blur and this is so stupid. maybe I should just make a pass for tonemapping before the blur pass
		//	postprocShader.use();
		//	postprocFlags.flipFlag(PostProcFlag_bloom);
		//	postprocFlags.set(postprocShader, "flags");
		//
		//	shader.use();
		//	shader.set1b("bloomOn", tempBool);
		//
		//	PBRShader.use();
		//	PBRShader.set1b("bloomOn", tempBool);
		//
		//	backgroundShader.use();
		//	backgroundShader.set1b("bloomOn", tempBool);
		//}
		//if(Checkbox("MSAA(doesn't work with deferred shading|NOT WORKING|)", &MSAA)){
		//	if (MSAA) glEnable(GL_MULTISAMPLE);
		//	else glDisable(GL_MULTISAMPLE);
		//}
		//Todo:: implement other MSAA options, FXAA and such. Motion blur, blur, bloom and other.
	}
	//if (CollapsingHeader("Options|NOT WORKING|")) {
	//}
	if (CollapsingHeader("Profiling", ImGuiTreeNodeFlags_DefaultOpen)) {
		Text(("IMGUI Average framerate: " + std::to_string((int)io.Framerate) + " FPS").c_str());
		Text(("IMGUI Average frametime: " + std::to_string(1000 / io.Framerate) + " ms").c_str());
		NewLine();

		Text("GPU Profiling");
		double frametime = (renderPass.result + guiPass.result + postprocPass.result) / 1000000.0;
		Text(("Query framerate: " + std::to_string((int)1000/frametime) + " FPS").c_str());
		Text(("Query frametime: " + std::to_string(frametime) + " ms").c_str());
		NewLine();

		
		Text(("Render Pass:		" + std::to_string(renderPass.result / 1000000.0) + "  ms").c_str());
		Text(("GUI Pass:		" + std::to_string(guiPass.result / 1000000.0) + "  ms").c_str());
		Text(("Post-Proc Pass:	" + std::to_string(postprocPass.result / 1000000.0) + "  ms").c_str());
	}
	if (CollapsingHeader("About")) {
		unsigned int windowWidth = GetWindowWidth();
		TextWrapped("Render Engine made with OGL and C++ by Alexander Atanasov.", windowWidth);
		
		NewLine();

		TextWrapped("Email: pharosisdoingnothing@gmail.com", windowWidth);
		TextWrapped("LinkedIn: Alexander Atanasov", windowWidth);
		NewLine();

		TextWrapped("I want to give credit to Joey DeVries, the author of 'LearnOpenGL,' for creating such a masterpiece that helped many in their journey in computer graphics.", windowWidth);
	}

	End();
	ImGui::ShowMetricsWindow();
	ImGui::ShowDemoWindow();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(GetDrawData());

	guiPass.end();
}
int initLibs() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_SAMPLES, 4);

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true); //TODO: this should be removed if I want performance

	#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	#endif

	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "GLRenderEngine", NULL, NULL); //Change the first null to glfwGetPrimaryMonitor() for fullscreen

	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { //Normal GLAD
	if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) { //GLAD2
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	return 0;
}
void initOGL(){
	int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, nullptr);
		//glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		glDebugMessageControl(GL_DEBUG_SOURCE_API,
			GL_DEBUG_TYPE_ERROR,
			GL_DEBUG_SEVERITY_HIGH,
			0, nullptr, GL_TRUE);
	}

	//Configure GL options
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //or GL_LINE
	glEnable(GL_DEPTH_TEST);

	//Face cull
	if (cullFace) glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	//MSAA
	if (MSAA) glEnable(GL_MULTISAMPLE);

	//Gamma correction
	//if (glGamma) glEnable(GL_FRAMEBUFFER_SRGB);

	//Remove visible edges of cube maps (Todo: this doesnt work for some cubemaps
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	//Input setup
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);

	//Background Color
	glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.f);
}
void initBloom() {
	glGenFramebuffers(1, &bloomFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO);
	glGenTextures(2, colorBuffers);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL
		);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// attach texture to framebuffer
		glFramebufferTexture2D(
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0
		);
	}

	unsigned int rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int bloomAttachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, bloomAttachments);
	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//PingPong
	glGenFramebuffers(2, pingpongFBO);
	glGenTextures(2, pingpongBuffers);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, pingpongBuffers[i]);
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL
		);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffers[i], 0
		);
	}
	blurShader.use();
	blurShader.set1i("image", 0);

	postprocShader.use();
	postprocShader.set1i("bloomBlur", 1);

	shader.use();
	shader.set1b("bloomOn", postprocFlags.hasFlag(PostProcFlag_bloom));

	PBRShader.use();
	PBRShader.set1b("bloomOn", postprocFlags.hasFlag(PostProcFlag_bloom));

	hdrSkyboxShader.use();
	hdrSkyboxShader.set1b("bloomOn", postprocFlags.hasFlag(PostProcFlag_bloom));
}
void initPostProc() {
	//Post process
	glGenFramebuffers(1, &postprocFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, postprocFBO);

	glGenTextures(1, &postprocColorBuffer);
	glBindTexture(GL_TEXTURE_2D, postprocColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postprocColorBuffer, 0);

	glGenRenderbuffers(1, &postprocRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, postprocRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, postprocRBO);
	// Check for errors buffers
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	postprocShader.use();
	postprocFlags.set(postprocShader, "flags");

	postprocShader.set1f("exposure", exposure);
	postprocShader.set1i("hdrBuffer", 0);
	postprocShader.set1f("gamma", gamma);
	postprocShader.set1f("maxRadiance", maxRadiance);
	postprocShader.set1i("tonemapMode", tonemapMode);

	//SRGB
	PBRShader.use();
	PBRShader.set1b("transformSRGB", transformSRGB);

	shader.use();
	shader.set1b("transformSRGB", transformSRGB);
}
void initDeferredShading() {

	/*Deferred rendering*/
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	// - position color buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

	// - normal color buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

	// - color + specular color buffer
	glGenTextures(1, &gAlbedoSpec);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

	// - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int gAttachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, gAttachments);
	unsigned int deferredRBO;
	glGenRenderbuffers(1, &deferredRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, deferredRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, deferredRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	deferredShader.use();
	deferredShader.set1b("renderQuad", false);

	shader.use();
	shader.set1i("deferredState", deferredState);
	shader.set1i("positionBuffer", 0);
	shader.set1i("normalBuffer", 1);
	shader.set1i("albedoBuffer", 2);

}
void initPBRVariables(){
	PBRShader.use();

	//Init
	PBRSkybox = ClassicMesh(skyboxVerts);

	if (currentSkybox == 0)
		hdrTexture = HDRMap("Images/abandoned_tiled_room_2k.hdr");
	if (currentSkybox == 1)
		hdrTexture = HDRMap("Images/abandoned_tiled_room_4k.hdr");
	if (currentSkybox == 2)
		hdrTexture = HDRMap("Images/HDR_029_Sky_Cloudy_Ref.hdr");
	if (currentSkybox == 3)
		hdrTexture = HDRMap("Images/thatch_chapel_2k.hdr");
	if (currentSkybox == 4)
		hdrTexture = HDRMap("Images/thatch_chapel_4k.hdr");

	//Set uniforms
	PBRShader.use();
	PBRShader.setMat4("proj", proj);
	PBRShader.setMat4("model", glm::mat4(1.f));

	pointLight.set(PBRShader, "pointLights[0]");
	dirLight.set(PBRShader, "dirLight");
	spotLight.set(PBRShader, "spotLight");

	PBRShader.set1b("pointLightEnabled", pointLightEnabled);
	PBRShader.set1b("dirLightEnabled", dirLightEnabled);
	PBRShader.set1b("spotLightEnabled", spotLightEnabled);

	PBRShader.set1b("iblSpecularity", iblSpecularity);

	PBRShader.set1i("irradianceMap", 5);
	PBRShader.set1i("prefilterMap", 6);
	PBRShader.set1i("brdfLUT", 7);

	PBRShader.set1b("useAlbedo", useAlbedo);
	PBRShader.set1b("useNormalMap", useNormalMap);
	PBRShader.set1b("useMetallic", useMetallic);
	PBRShader.set1b("useRoughness", useRoughness);
	PBRShader.set1b("useAlbedo", useAmbientMap);

	hdrSkyboxShader.use();
	hdrSkyboxShader.setMat4("proj", proj);

	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	equirectangularToCubemapShader.use();
	equirectangularToCubemapShader.set1i("equirectangularMap", 0);
	equirectangularToCubemapShader.setMat4("proj", captureProjection);

	irradianceShader.use();
	irradianceShader.set1i("environmentMap", 0);
	irradianceShader.setMat4("proj", captureProjection);

	prefilterShader.use();
	prefilterShader.set1i("environmentMap", 0);
	prefilterShader.setMat4("proj", captureProjection);

	//FBO
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	//Environment Cubemap init
	glGenTextures(1, &envCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	for (unsigned int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 1024, 1024, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//Irradiance Cubemap init
	glGenTextures(1, &irradianceMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);

	for (unsigned int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
	
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//Prefilter Cubemap init
	glGenTextures(1, &prefilterMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);

	for (unsigned int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	//BRDF calculation
	glGenTextures(1, &brdfLUTTexture);

	glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

	glViewport(0, 0, 512, 512);

	brdfShader.use();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderQuad.Draw(brdfShader, {});

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// then before rendering, configure the viewport to the original framebuffer's screen dimensions
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
}
void updateIBL() {
	//Convert equirectangular HDR texture to a Cubemap
	equirectangularToCubemapShader.use();
	hdrTexture.bind(0);

	glViewport(0, 0, 1024, 1024);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 1024, 1024);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

	for (unsigned int i = 0; i < 6; ++i){
		equirectangularToCubemapShader.setMat4("view", captureViews[i]);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		PBRSkybox.Draw(equirectangularToCubemapShader);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Calculate convolution for irradiance cubemap
	glViewport(0, 0, 32, 32);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

	irradianceShader.use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

	for (unsigned int i = 0; i < 6; ++i){
		irradianceShader.setMat4("view", captureViews[i]);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		PBRSkybox.Draw(irradianceShader);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Quasi monte-carlo simulation for prefilter cubemap
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

	prefilterShader.use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

	unsigned int maxMipLevels = 5;
	for (unsigned int mip = 0; mip < maxMipLevels; ++mip) {
		// reisze framebuffer according to mip-level size.
		unsigned int mipWidth = 128 * std::pow(0.5, mip);
		unsigned int mipHeight = 128 * std::pow(0.5, mip);

		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);

		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = (float)mip / (float)(maxMipLevels - 1);

		prefilterShader.set1f("roughness", roughness);

		for (unsigned int i = 0; i < 6; ++i) {
			prefilterShader.setMat4("view", captureViews[i]);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			PBRSkybox.Draw(prefilterShader);
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Revert framebuffer default screen dimentions
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
}
void initImGui() {
	IMGUI_CHECKVERSION();
	CreateContext();
	//ImGuiIO& io = GetIO(); (void)io;
	//StyleColorsDark();
	StyleColorsClassic();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
	style = &GetStyle();
	windowAlpha = style->Alpha;

	style->Alpha = .3f;
}
void updateModelMatrices() {
	modelMat = glm::scale(glm::mat4(1.f), modelScale);
	modelMat = glm::translate(modelMat, modelPos);

	if (modelRot != glm::vec3(0.f) && modelRotAngle != 0.f) rotationMat = glm::rotate(glm::mat4(1.f), glm::radians(modelRotAngle), modelRot);
	else rotationMat = glm::mat4(1.f);
}
void updateModelTransforms() {
	switch (currentModel) {
	case 0:
		modelPtr = &gun;

		modelScale = glm::vec3(0.04f);
		modelRot = glm::vec3(1.f, 0.f, 0.f);
		modelRotAngle = 270.f;

		break;
	case 1:
		modelPtr = &suzanne;

		modelScale = glm::vec3(1.f);
		modelRot = glm::vec3(0.f);
		modelRotAngle = 0.f;

		break;
	case 2:
		modelPtr = &backpack;

		modelScale = glm::vec3(1.f);
		modelRot = glm::vec3(1.f, 0.f, 0.f);
		modelRotAngle = 270.f;

		break;
	}

	applyMaterial();
	updateModelMatrices();
}
void updateMaterial() {
	switch (materialState) {
	case 0:
		material.loadTextures("Images/Gold/albedo.png", "Images/Gold/normal.png", "Images/Gold/metallic.png", "Images/Gold/roughness.png", "Images/Gold/ao.png");
		break;
	case 1:
		material.loadTextures("Images/Grass/albedo.png", "Images/Grass/normal.png", "Images/Grass/metallic.png", "Images/Grass/roughness.png", "Images/Grass/ao.png");
		break;
	case 2:
		material.loadTextures("Images/plastic/albedo.png", "Images/plastic/normal.png", "Images/plastic/metallic.png", "Images/plastic/roughness.png", "Images/plastic/ao.png");
		break;
	case 3:
		material.loadTextures("Images/rusted_iron/albedo.png", "Images/rusted_iron/normal.png", "Images/rusted_iron/metallic.png", "Images/rusted_iron/roughness.png", "Images/rusted_iron/ao.png");
		break;
	case 4:
		material.loadTextures("Images/Wall/albedo.png", "Images/Wall/normal.png", "Images/Wall/metallic.png", "Images/Wall/roughness.png", "Images/Wall/ao.png");
		break;
	default:
		modelPtr->meshes[0].currentMaterial = &modelPtr->meshes[0].material;
	}

	if(materialState != 5)
		modelPtr->meshes[0].currentMaterial = &material;
}
void applyMaterial() {
	if (materialState == 5)
		modelPtr->meshes[0].currentMaterial = &modelPtr->meshes[0].material;
	else
		modelPtr->meshes[0].currentMaterial = &material;
}
void initModels() {
	gun.loadModel("Objects/Cerberus_by_Andrew_Maximov/Cerberus_LP.FBX");
	gun.meshes[0].material.loadTextures("Objects/Cerberus_by_Andrew_Maximov/Textures/Cerberus_A.tga", "Objects/Cerberus_by_Andrew_Maximov/Textures/Cerberus_N.tga", "Objects/Cerberus_by_Andrew_Maximov/Textures/Cerberus_M.tga", "Objects/Cerberus_by_Andrew_Maximov/Textures/Cerberus_R.tga", "Objects/Cerberus_by_Andrew_Maximov/Textures/Raw/Cerberus_AO.tga");

	//suzanne.loadModel("Objects/suzanne/scene.gltf");
	//suzanne.meshes[0].material.loadTextures("Images/White.png", "", "", "Images/White.png", "Images/White.png");
	//
	//backpack.loadModel("Objects/SurvivalBackpack/Survival_BackPack_2.fbx");
	//backpack.meshes[0].material.loadTextures("Objects/SurvivalBackpack/albedo.jpg", "Objects/SurvivalBackpack/normal.png", "Objects/SurvivalBackpack/metallic.jpg", "Objects/SurvivalBackpack/roughness.jpg", "Objects/SurvivalBackpack/AO.jpg");
}
void setupMainLoop() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	processInput(window);
	if (mouseLocked)
		cam.processInput(window);

	//Setup IMGUI
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	NewFrame();

	io = GetIO();

	cam.updateView();
	view = cam.getView();

	if (demoRotation) modelDemoRot = glm::rotate(modelDemoRot, glm::radians(-0.1f), glm::vec3(0.f, 1.f, 0.f));
	bindUniforms(shader);
	bindUniforms(PBRShader);
}
void bindUniforms(Shader& shader) {
	shader.use();
	shader.setMat4("view", view);

	shader.setVec3("viewPos", cam.getPos());
	if (spotLightEnabled) {
		shader.setVec3("spotLight.position", cam.getPos());
		shader.setVec3("spotLight.direction", cam.camFront);
	}

	shader.setMat4("model", modelMat * modelDemoRot * rotationMat);
}