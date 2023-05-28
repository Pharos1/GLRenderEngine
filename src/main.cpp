/*
	
	I want to give credit to Joey DeVries, the author of 'LearnOpenGL,' for creating such a masterpiece that helped many in their journey in computer graphics.
	
*/

#include <GLEW/glew.h>
#include <GLAD/gl.h>
#include <GLFW/glfw3.h>
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <iostream>
#include <filesystem>

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
#include<thread>
#include<chrono>

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

//On Application Start
int setupDependencies();
void initBloom();
void initPostProc();
void initMSAA();
void initDeferredShading();
void setupPBR();
void initImGui();
void loadModels();

//On state change(buttons, resize, keys, etc.)

	//--UI
void updateIBL();
void updateModelMatrices();
void updateObjectMatrices();
void updateCurrentModel();
void updateMaterial();

	//--Window and OS
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void setupBloom();
void setupPostProc();
void setupMSAA();
void setupDeferredShading();

//Every frame
void processInput(GLFWwindow* window);
void updateUniforms(Shader& shader);
void renderScene(Shader& shader, Shader& PBRShader);
void beginPostProcess();
void endPostProcess();
void updateImGui();

//Window & projection
GLFWwindow* window;
unsigned int SCR_WIDTH = 920; //1280
unsigned int SCR_HEIGHT = 720; //720

float fov = 45.f;
glm::mat4 model(1.f);
glm::mat4 view(1.f);
glm::mat4 proj(1.f);

RenderQuad renderQuad;

Camera cam(glm::vec3(0.f, 0.5f, 3.f), 7.5f);
bool mouseLocked = true;

std::string openGLVersion;
std::string glslVersion;
std::string vendor;
std::string gpuVersion;

//Shaders
Shader shader;
//Shader skyboxShader;
Shader deferredShader;
Shader postprocShader;
Shader blurShader;
Shader debugQuadShader;
Shader PBRShader;
Shader equirectangularToCubemapShader;
Shader hdrSkyboxShader;
Shader irradianceShader;
Shader prefilterShader;
Shader brdfShader;
Shader lightBoxShader;

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
//PostProcFlags postprocFlags(PostProcFlag_gammaCorrection);

int msaaSampleCount = 4;

float exposure = 2.2f;
float gamma = 2.2f;

unsigned int bloomFBO;
unsigned int bloomRBO;
unsigned int colorBuffers[2];

unsigned int pingpongFBO[2];
unsigned int pingpongBuffers[2];

unsigned int msaaFBO;
unsigned int msaaRBO;
unsigned int msaaColorBuffer;
unsigned int intermediateFBO;
unsigned int msaaTexture;

//Lights
bool dirLightEnabled = true;
bool pointLightEnabled = false;
bool spotLightEnabled = false;

DirLight dirLight(glm::vec3(-1.f, -1.f, -1.f), glm::vec3(1.f), 1.f);//(glm::vec3(-1.f, -1.f, -1.f), glm::vec3(.1f), glm::vec3(1.f), glm::vec3(.3f));
PointLight pointLight(glm::vec3(0.f, .5f, 3.f), glm::vec3(1.f), 1.f);//(glm::vec3(0.f, .5f, 3.f), glm::vec3(.05f), glm::vec3(1.f), glm::vec3(.3f), 1.f, .014f, .000007f);
SpotLight spotLight(cam.getPos(), cam.camFront, glm::vec3(1.f), 1.f, 12.5f, 15.f);//(cam.getPos(), cam.camFront, glm::vec3(0.f), glm::vec3(1.f), glm::vec3(.3f), 1.f, .09f, .032f, 12.5f, 15.f);

//Deferred shading
unsigned int gBuffer;
unsigned int gPosition, gNormal, gAlbedoSpec;
unsigned int deferredRBO;

//PBR & IBL
HDRMap hdrTexture;
HDRSkybox PBRSkybox;
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
bool iblEnabled = true;
int materialState = 5;
bool demoRotation = false;
bool transformSRGB = true;
bool useAlbedo = true;
bool useNormalMap = true;
bool useMetallic = true;
bool useRoughness = true;
bool useAmbientMap = true;
int currentSkybox = 0;
int currentModel = 1;
int tonemapMode = 5;
float maxRadiance = 1.f;
int antiAliasing = 0;
bool deferredShadingEnabled = false;
int deferredState = 4;
bool vsyncOn = false;
float bgAlpha = .7f;
int currentStyle = 1;
bool gammaOn = true;
bool bloomOn = false;
bool blurOn = false;

//FXAA
#define DEFAULT_EDGE_THRESHOLD_MIN .0312f;
#define DEFAULT_EDGE_THRESHOLD_MAX .125f;
#define DEFAULT_ITERATIONS 12;
#define DEFAULT_SUBPIXEL_QUALITY .25f;
bool fxaaRevert = false;
float EDGE_THRESHOLD_MIN = .0312f;
float EDGE_THRESHOLD_MAX = .125f;
int ITERATIONS = 12;
float SUBPIXEL_QUALITY = .25f;

//Scene
Model gun;
Model suzanne;
Model backpack;
Model* modelPtr;

Material material;

glm::vec3 modelPos(0.f);
glm::vec3 modelScale(1.f);
glm::vec3 modelRot(0.f);

glm::vec3 objectPos(0.f);
glm::vec3 objectScale(1.f);
glm::vec3 objectRot(0.f);

glm::mat4 modelScaleMat(1.f);
glm::mat4 modelTransMat(1.f);
glm::mat4 modelRotMat(1.f);
glm::mat4 modelRotScaleMat(1.f);
glm::mat4 objectScaleMat(1.f);
glm::mat4 objectTransMat(1.f);
glm::mat4 objectRotMat(1.f);

glm::mat4 objectDemoRot(1.f);

float demoRotSpeed = .75f;

//Query
Query guiPass;
Query renderPass;
Query postprocPass;

//Light box
ClassicMesh lightBox;

int main() {
	//Change the current path (in case the file is run outside the IDE). Also this should be changed if i would release a seperate built .exe file(Now the current dir is being set to the solution path.
	std::filesystem::current_path(std::filesystem::path(__FILE__).parent_path().parent_path()); //The solution path
	
	if(setupDependencies()) return -1;

	//Shaders
	shader.loadShader("Shaders/main.vert", "Shaders/main.frag");
	//skyboxShader = Shader("Shaders/skybox.vert", "Shaders/skybox.frag");
	deferredShader.loadShader("Shaders/main.vert", "Shaders/deferred.frag");
	postprocShader.loadShader("Shaders/renderQuad.vert", "Shaders/postProc.frag");
	blurShader.loadShader("Shaders/renderQuad.vert", "Shaders/blur.frag");
	debugQuadShader.loadShader("Shaders/renderQuad.vert", "Shaders/renderQuad.frag");
	PBRShader.loadShader("Shaders/PBR/PBR.vert", "Shaders/PBR/PBR.frag");
	equirectangularToCubemapShader.loadShader("Shaders/cubemap.vert", "Shaders/PBR/EquirectangularToCubemap.frag");
	hdrSkyboxShader.loadShader("Shaders/skybox.vert", "Shaders/PBR/hdrSkybox.frag");
	irradianceShader.loadShader("Shaders/cubemap.vert", "Shaders/PBR/irradianceConvolution.frag");
	prefilterShader.loadShader("Shaders/cubemap.vert", "Shaders/PBR/prefilter.frag");
	brdfShader.loadShader("Shaders/renderQuad.vert", "Shaders/PBR/brdfShader.frag");
	lightBoxShader.loadShader("Shaders/lightBox.vert", "Shaders/lightBox.frag");

	//Matrices
	proj = glm::perspective(glm::radians(fov), SCR_WIDTH / (float)SCR_HEIGHT, .1f, 100.f);

	shader.use();
	shader.setMat4("proj", proj);

	renderQuad = RenderQuad(quadVertices);

	//Lights
	dirLight.set(shader, "dirLight");
	pointLight.set(shader, "pointLights[0]");
	spotLight.set(shader, "spotLight");

	shader.set1b("dirLightEnabled", dirLightEnabled);
	shader.set1b("pointLightEnabled", pointLightEnabled);
	shader.set1b("spotLightEnabled", spotLightEnabled);

	lightBoxShader.use();
	lightBoxShader.setVec3("pos", pointLight.pos);
	lightBoxShader.setVec3("diffuse", pointLight.diffuse);

	initBloom();
	initDeferredShading();
	initPostProc();
	initMSAA();

	setupPBR();
	updateIBL();

	loadModels();
	updateCurrentModel();

	updateMaterial();

	//IMGUI
	initImGui();

	//Queries
	guiPass.loadQuery(GL_TIME_ELAPSED);
	renderPass.loadQuery(GL_TIME_ELAPSED);
	postprocPass.loadQuery(GL_TIME_ELAPSED);

	//Software & Hardware Info
	openGLVersion = (char*)glGetString(GL_VERSION);
	glslVersion = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
	vendor = (char*)glGetString(GL_VENDOR);
	gpuVersion = (char*)glGetString(GL_RENDERER);

	//Light box
	lightBox = ClassicMesh(cubeVertices);

	while (!glfwWindowShouldClose(window)) {
		//glCheckError();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				
		if (deferredShadingEnabled) {
			deferredShader.use();
			glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
			deferredShader.setMat4("proj", proj);

			deferredShader.use();
			modelPtr->Draw(deferredShader);
		
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			beginPostProcess();

			shader.use();

			shader.set1b("deferredEnabled", true);
			renderQuad.Draw(shader, { gPosition, gNormal, gAlbedoSpec }, 5);
			shader.set1b("deferredEnabled", false);
		}
		else {
			//Begin MSAA
			if (antiAliasing == 1) {
				glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			}
			else
				beginPostProcess();

			renderScene(shader, PBRShader);
		}

		if (pointLightEnabled)
			lightBox.Draw(lightBoxShader);


		if (!deferredShadingEnabled && antiAliasing == 1) { //MSAA doesn't work with deferred rendering
			//End MSAA
			glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO);
			glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			//Post-processing
			beginPostProcess();
			debugQuadShader.use();
			renderQuad.Draw(debugQuadShader, { msaaTexture });
		}
		endPostProcess();

		updateImGui();

		//Small optimization here! Get io and do some math in the end of the frame
		dt.updateDT();

		processInput(window);
		if (mouseLocked)
			cam.processInput(window);

		cam.updateView();
		view = cam.getView();

		if (demoRotation)
			objectDemoRot = glm::rotate(objectDemoRot, demoRotSpeed * dt.deltaTime, glm::vec3(0.f, 1.f, 0.f));
		model = modelTransMat * objectDemoRot * modelRotScaleMat;

		lightBoxShader.use();
		lightBoxShader.setMat4("PVMat", proj * view);

		updateUniforms(shader);
		updateUniforms(PBRShader);
		updateUniforms(deferredShader);

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

//On Application Start
int setupDependencies() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); //3.3 Cuz it runs everywhere
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//glfwWindowHint(GLFW_SAMPLES, 4);

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

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { //Normal GLAD
		//if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) { //GLAD2
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	GLenum err = glewInit();
	if (err != GLEW_OK) {
		/* Problem: glewInit failed, something is seriously wrong. */
		std::cout << "Glew Error: " << glewGetErrorString(err) << std::endl;
		return -1;
	}
	glfwSwapInterval(vsyncOn);
	
	//Debugging
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
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	//Remove visible edges of cube maps (Todo: this doesnt work for some cubemaps
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	//Input setup
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);

	//Background Color
	glClearColor(0.f, 0.f, 0.f, 1.f);
	
	return 0;
}
void initBloom() {
	glGenFramebuffers(1, &bloomFBO);
	glGenRenderbuffers(1, &bloomRBO);
	glGenFramebuffers(2, pingpongFBO);
	glGenTextures(2, pingpongBuffers);
	glGenTextures(2, colorBuffers);
	setupBloom();
}
void initPostProc() {
	glGenFramebuffers(1, &postprocFBO);
	glGenTextures(1, &postprocColorBuffer);
	glGenRenderbuffers(1, &postprocRBO);
	setupPostProc();
}
void initMSAA() {
	glGenFramebuffers(1, &msaaFBO);
	glGenTextures(1, &msaaColorBuffer);
	glGenRenderbuffers(1, &msaaRBO);
	glGenFramebuffers(1, &intermediateFBO);
	glGenTextures(1, &msaaTexture);
	setupMSAA();
}
void initDeferredShading() {
	glGenFramebuffers(1, &gBuffer);
	glGenTextures(1, &gPosition);
	glGenTextures(1, &gNormal);
	glGenTextures(1, &gAlbedoSpec);
	glGenRenderbuffers(1, &deferredRBO);
	setupDeferredShading();
}
void setupPBR() {
	PBRShader.use();

	//Init
	PBRSkybox.setup();
	PBRSkybox.texturePtr = &hdrTexture;

	if (currentSkybox == 0)
		hdrTexture.loadHDRMap("Images/HDRI/abandoned_tiled_room_2k.hdr");
	else if (currentSkybox == 1)
		hdrTexture.loadHDRMap("Images/HDRI/abandoned_tiled_room_4k.hdr");
	else if(currentSkybox == 2)
		hdrTexture.loadHDRMap("Images/HDRI/HDR_029_Sky_Cloudy_Ref.hdr");
	else if(currentSkybox == 3)
		hdrTexture.loadHDRMap("Images/HDRI/thatch_chapel_2k.hdr");
	else if (currentSkybox == 4)
		hdrTexture.loadHDRMap("Images/HDRI/thatch_chapel_4k.hdr");

	//Set uniforms
	PBRShader.use();
	PBRShader.setMat4("proj", proj);
	PBRShader.setMat4("model", glm::mat4(1.f));

	dirLight.set(PBRShader, "dirLight");
	pointLight.set(PBRShader, "pointLights[0]");
	spotLight.set(PBRShader, "spotLight");

	PBRShader.set1b("pointLightEnabled", pointLightEnabled);
	PBRShader.set1b("dirLightEnabled", dirLightEnabled);
	PBRShader.set1b("spotLightEnabled", spotLightEnabled);

	PBRShader.set1b("iblEnabled", iblEnabled);

	PBRShader.set1b("useAlbedo", useAlbedo);
	PBRShader.set1b("useNormalMap", useNormalMap);
	PBRShader.set1b("useMetallic", useMetallic);
	PBRShader.set1b("useRoughness", useRoughness);
	PBRShader.set1b("useAlbedo", useAmbientMap);

	hdrSkyboxShader.use();
	hdrSkyboxShader.setMat4("proj", proj);

	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	equirectangularToCubemapShader.use();
	equirectangularToCubemapShader.setMat4("proj", captureProjection);

	irradianceShader.use();
	irradianceShader.setMat4("proj", captureProjection);

	prefilterShader.use();
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


	//Clean up
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
void initImGui() {
	IMGUI_CHECKVERSION();
	CreateContext();
	//ImGuiIO& io = GetIO(); (void)io;
	//StyleColorsDark();
	
	//Style
	if (currentStyle == 0) StyleColorsClassic();
	else if (currentStyle == 1) StyleColorsDark();
	else if (currentStyle == 2) StyleColorsLight();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
	style = &GetStyle();
	windowAlpha = style->Alpha;

	style->Alpha = .3f;
}
void loadModels() {
	//gun.loadModel("Objects/Cerberus_by_Andrew_Maximov/Cerberus_LP.FBX");
	//gun.meshes[0].material.loadTextures("Objects/Cerberus_by_Andrew_Maximov/Textures/Cerberus_A.tga", "Objects/Cerberus_by_Andrew_Maximov/Textures/Cerberus_N.tga", "Objects/Cerberus_by_Andrew_Maximov/Textures/Cerberus_M.tga", "Objects/Cerberus_by_Andrew_Maximov/Textures/Cerberus_R.tga", "Objects/Cerberus_by_Andrew_Maximov/Textures/Raw/Cerberus_AO.tga");

	suzanne.loadModel("Objects/suzanne/scene.gltf");
	suzanne.meshes[0].material.loadTextures("Images/White.png", "", "", "Images/White.png", "Images/White.png");
	
	//backpack.loadModel("Objects/SurvivalBackpack/Survival_BackPack_2.fbx");
	//backpack.meshes[0].material.loadTextures("Objects/SurvivalBackpack/albedo.jpg", "Objects/SurvivalBackpack/normal.png", "Objects/SurvivalBackpack/metallic.jpg", "Objects/SurvivalBackpack/roughness.jpg", "Objects/SurvivalBackpack/AO.jpg");
}

//On state change(buttons, resize, keys, etc.)
	//--UI
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
void updateModelMatrices() {
	modelScaleMat = objectScaleMat * glm::scale(glm::mat4(1.f), modelScale);
	modelTransMat = objectTransMat * glm::translate(glm::mat4(1.f), modelPos);
	if (modelRot != glm::vec3(0.f)) {
		modelRotMat = glm::rotate(glm::mat4(1.f), glm::radians(modelRot.x), glm::vec3(1.f, 0.f, 0.f));
		modelRotMat = glm::rotate(modelRotMat, glm::radians(modelRot.y), glm::vec3(0.f, 1.f, 0.f));
		modelRotMat = glm::rotate(modelRotMat, glm::radians(modelRot.z), glm::vec3(0.f, 0.f, 1.f));
	}
	else
		modelRotMat = glm::mat4(1.f);
	modelRotMat = objectRotMat * modelRotMat;
	modelRotScaleMat = modelRotMat * modelScaleMat; //Optimization
}
void updateObjectMatrices() {
	objectScaleMat = glm::scale(glm::mat4(1.f), objectScale);
	objectTransMat = glm::translate(glm::mat4(1.f), objectPos);
	if (objectRot != glm::vec3(0.f)) {
		objectRotMat = glm::rotate(glm::mat4(1.f), glm::radians(objectRot.x), glm::vec3(1.f, 0.f, 0.f));
		objectRotMat = glm::rotate(objectRotMat, glm::radians(objectRot.y), glm::vec3(0.f, 1.f, 0.f));
		objectRotMat = glm::rotate(objectRotMat, glm::radians(objectRot.z), glm::vec3(0.f, 0.f, 1.f));
	}
	else
		objectRotMat = glm::mat4(1.f);
	updateModelMatrices();
}
void updateCurrentModel() {
	switch (currentModel) {
	case 0:
		modelPtr = &gun;

		modelPos = glm::vec3(0.f);
		modelScale = glm::vec3(0.02f);
		modelRot = glm::vec3(270.f, 0.f, 0.f);
		break;
	case 1:
		modelPtr = &suzanne;

		modelPos = glm::vec3(0.f);
		modelScale = glm::vec3(1.f);
		modelRot = glm::vec3(0.f);
		break;
	case 2:
		modelPtr = &backpack;

		modelPos = glm::vec3(0.f);
		modelScale = glm::vec3(1.f);
		modelRot = glm::vec3(270.f, 0.f, 0.f);
		break;
	}

	updateMaterial();
	updateObjectMatrices();
}
void updateMaterial() {
	if (materialState == 0)
		material.loadTextures("Images/Gold/albedo.png", "Images/Gold/normal.png", "Images/Gold/metallic.png", "Images/Gold/roughness.png", "Images/Gold/ao.png");
	if (materialState == 1)
		material.loadTextures("Images/Grass/albedo.png", "Images/Grass/normal.png", "Images/Grass/metallic.png", "Images/Grass/roughness.png", "Images/Grass/ao.png");
	if (materialState == 2)
		material.loadTextures("Images/plastic/albedo.png", "Images/plastic/normal.png", "Images/plastic/metallic.png", "Images/plastic/roughness.png", "Images/plastic/ao.png");
	if (materialState == 3)
		material.loadTextures("Images/rusted_iron/albedo.png", "Images/rusted_iron/normal.png", "Images/rusted_iron/metallic.png", "Images/rusted_iron/roughness.png", "Images/rusted_iron/ao.png");
	if (materialState == 4)
		material.loadTextures("Images/Wall/albedo.png", "Images/Wall/normal.png", "Images/Wall/metallic.png", "Images/Wall/roughness.png", "Images/Wall/ao.png");

	//Apply the material
	if (modelPtr->meshes.size() == 0) { //If it has no meshes just print an error
		std::cout << "ERROR::MAIN.H::UPDATE_MATERIAL::MODEL HAS NO MESHES!\n";
		return;
	}

	if (materialState == 5)
		modelPtr->meshes[0].currentMaterial = &modelPtr->meshes[0].material;
	else
		modelPtr->meshes[0].currentMaterial = &material;
}

	//--Window and OS
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

	hdrSkyboxShader.use();
	hdrSkyboxShader.setMat4("proj", proj);
	
	//Update effects' frame buffer
	setupBloom();
	setupPostProc();
	setupDeferredShading();
	setupMSAA();
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (mouseLocked)
		cam.processMouse(xpos, ypos);
	else {
		cam.lastX = xpos; //DO this so the mouse dont skip when entering in view mode
		cam.lastY = ypos;
	}
}
void setupBloom() {
	glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO);
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

	glBindRenderbuffer(GL_RENDERBUFFER, bloomRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, bloomRBO);
	// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int bloomAttachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, bloomAttachments);
	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//PingPong
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
		glActiveTexture(GL_TEXTURE0);
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
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	shader.use();
	shader.set1b("bloomOn", bloomOn);

	PBRShader.use();
	PBRShader.set1b("bloomOn", bloomOn);

	hdrSkyboxShader.use();
	hdrSkyboxShader.set1b("bloomOn", bloomOn);
}
void setupPostProc() {
	//Post process
	glBindFramebuffer(GL_FRAMEBUFFER, postprocFBO);

	glBindTexture(GL_TEXTURE_2D, postprocColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postprocColorBuffer, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, postprocRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, postprocRBO);
	
	//Check for errors
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	postprocShader.use();
	postprocShader.set1b("gammaOn", gammaOn);
	postprocShader.set1b("bloomOn", bloomOn);
	postprocShader.set1b("blurOn", blurOn);

	postprocShader.set1f("exposure", exposure);
	postprocShader.set1f("gamma", gamma);
	postprocShader.set1f("maxRadiance", maxRadiance);
	postprocShader.set1i("tonemapMode", tonemapMode);

	//FXAA
	postprocShader.set1b("fxaaEnabled", antiAliasing == 2);
	postprocShader.setVec2("inverseScreenSize", glm::vec2(1.f / SCR_WIDTH, 1.f / SCR_HEIGHT));
	postprocShader.set1f("EDGE_THRESHOLD_MIN", EDGE_THRESHOLD_MIN);
	postprocShader.set1f("EDGE_THRESHOLD_MAX", EDGE_THRESHOLD_MAX);
	postprocShader.set1i("ITERATIONS", ITERATIONS);
	postprocShader.set1f("SUBPIXEL_QUALITY", SUBPIXEL_QUALITY);

	//SRGB
	PBRShader.use();
	PBRShader.set1b("transformSRGB", transformSRGB);

	shader.use();
	shader.set1b("transformSRGB", transformSRGB);
}
void setupMSAA() {
	glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

	//Multisampled texture
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaaColorBuffer);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, msaaSampleCount, GL_RGB, SCR_WIDTH, SCR_HEIGHT, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, msaaColorBuffer, 0);
	
	//Multisampled RBO
	glBindRenderbuffer(GL_RENDERBUFFER, msaaRBO);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, msaaSampleCount, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, msaaRBO);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Intermediate post-processing FBO
	glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);

	glBindTexture(GL_TEXTURE_2D, msaaTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, msaaTexture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		cout << "ERROR::FRAMEBUFFER:: Intermediate framebuffer is not complete!" << endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void setupDeferredShading() {
	/*Deferred rendering*/
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	// - position color buffer
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

	// - normal color buffer
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

	// - color + specular color buffer
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

	// - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int gAttachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, gAttachments);
	glBindRenderbuffer(GL_RENDERBUFFER, deferredRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, deferredRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	deferredShader.use();
	deferredShader.set1b("deferredEnabled", false);

	shader.use();
	shader.set1i("deferredState", deferredState);
	shader.set1b("deferredEnabled", false);

}

//Every frame
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
void updateUniforms(Shader& shader) {
	shader.use();
	shader.setMat4("view", view);

	shader.setVec3("viewPos", cam.getPos());
	if (spotLightEnabled) {
		shader.setVec3("spotLight.position", cam.getPos());
		shader.setVec3("spotLight.direction", cam.camFront);
	}

	shader.setMat4("model", model);
}
void renderScene(Shader& shader, Shader& PBRShader) {
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

	hdrSkyboxShader.use();
	hdrSkyboxShader.setMat4("view", view);
	PBRSkybox.Draw(hdrSkyboxShader);

	renderPass.end();
}
void beginPostProcess() {
	if (bloomOn)
		glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO);
	else
		glBindFramebuffer(GL_FRAMEBUFFER, postprocFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void endPostProcess() {
	postprocPass.begin();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (bloomOn) {
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
void updateImGui() {
	guiPass.begin();

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	NewFrame();

	io = GetIO();

	ImGuiStyle& style = GetStyle();

	SetNextWindowSizeConstraints(ImVec2(200, SCR_HEIGHT), ImVec2(SCR_WIDTH, SCR_HEIGHT));
	SetNextWindowBgAlpha(bgAlpha);

	Begin("Option Window", NULL, ImGuiWindowFlags_NoMove);
	SetWindowPos(ImVec2(SCR_WIDTH - GetWindowWidth(), 0));
	
	if (CollapsingHeader("Rendering", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (TreeNode("Objects")) {
			Checkbox("Demo Rotation", &demoRotation);
			SliderFloat("Speed", &demoRotSpeed, 0.f, 10.f);
			NewLine();

			bool updatePos = SliderFloat3("Position", glm::value_ptr(objectPos), -10.f, 10.f);
			bool updateScale = SliderFloat3("Scale", glm::value_ptr(objectScale), 0.f, 1.f);
			bool updateRot = SliderFloat3("Rotation", glm::value_ptr(objectRot), 0.f, 360.f);
			if (updatePos || updateScale || updateRot)
				updateObjectMatrices();
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
				if (Button("Apply"))
					updateMaterial();

				TreePop();
			}
			if (TreeNode("Model")) {
				std::vector<const char*> models = { "PBR Gun", "Suzanne", "Backpack"};

				PushItemWidth(GetWindowWidth() - 130);
				Combo("Current Model", &currentModel, models.data(), models.size());
				PopItemWidth();

				NewLine();
				if (Button("Apply"))
					updateCurrentModel();
				
				NewLine();
				bool updatePos = SliderFloat3("Local Position", glm::value_ptr(modelPos), -10.f, 10.f);
				bool updateScale = SliderFloat3("Local Scale", glm::value_ptr(modelScale), 0.f, 1.f);
				bool updateRot = SliderFloat3("Local Rotation", glm::value_ptr(modelRot), 0.f, 360.f);
				if (updatePos || updateScale || updateRot)
					updateModelMatrices();
				TreePop();
			}

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


			if (Checkbox("Use IBL", &iblEnabled))
				PBRShader.set1b("iblEnabled", iblEnabled);
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
		if (TreeNode("Deferred Shading/Rendering")) {
			Checkbox("Enable Deferred Shading", &deferredShadingEnabled); //Doesn't work with pbr
			BeginDisabled(!deferredShadingEnabled);

			bool updatePos  = RadioButton("Display Position Buffer", &deferredState, 0);
			bool updateNorm = RadioButton("Display Normal Buffer", &deferredState, 1);
			bool updateAlbed= RadioButton("Display Albedo Buffer", &deferredState, 2);
			bool updateSpec = RadioButton("Display Specular Buffer", &deferredState, 3);
			bool updateComb = RadioButton("Display Combined Buffer", &deferredState, 4);
			if (updatePos || updateNorm || updateAlbed || updateSpec || updateComb) {
				shader.use();
				shader.set1i("deferredState", deferredState);
			}
			EndDisabled();
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
			nodeOpened = TreeNodeEx("Dir Light");

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

				if (SliderFloat3("Dir##0", glm::value_ptr(dirLight.dir), -1.f, 1.f)) {
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
			nodeOpened = TreeNodeEx("Point Light");
			SameLine();
			if (Checkbox("##1", &pointLightEnabled)) {
				shader.use();
				shader.set1b("pointLightEnabled", pointLightEnabled);
				PBRShader.use();
				PBRShader.set1b("pointLightEnabled", pointLightEnabled);
			}

			BeginDisabled(!pointLightEnabled);
			if (nodeOpened) {
				if (ColorEdit3("Diffuse##1", glm::value_ptr(pointLight.diffuse))) {
					shader.use();
					shader.setVec3("pointLights[0].diffuse", pointLight.diffuse);
					PBRShader.use();
					PBRShader.setVec3("pointLights[0].diffuse", pointLight.diffuse);
				}

				if (SliderFloat3("Pos##1", glm::value_ptr(pointLight.pos), -10, 10)) {
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
			nodeOpened = TreeNodeEx("Spot Light");
			SameLine();
			if (Checkbox("##2", &spotLightEnabled)) {
				shader.use();
				shader.set1b("spotLightEnabled", spotLightEnabled);
				PBRShader.use();
				PBRShader.set1b("spotLightEnabled", spotLightEnabled);
			}

			BeginDisabled(!spotLightEnabled);
			if (nodeOpened) {
				if (ColorEdit3("Diffuse##2", glm::value_ptr(spotLight.diffuse))) {
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
		if (TreeNode("Skybox")) {
			if (RadioButton("Abandoned Room 2K", &currentSkybox, 0)) {
				hdrTexture.loadHDRMap("Images/HDRI/abandoned_tiled_room_2k.hdr");
				updateIBL();
			}
			if (RadioButton("Abandoned Room 4K", &currentSkybox, 1)) {
				hdrTexture.loadHDRMap("Images/HDRI/abandoned_tiled_room_4k.hdr");
				updateIBL();
			}
			if (RadioButton("Grass Field", &currentSkybox, 2)) {
				hdrTexture.loadHDRMap("Images/HDRI/HDR_029_Sky_Cloudy_Ref.hdr");
				updateIBL();
			}
			if (RadioButton("Thatch Chapel 2K", &currentSkybox, 3)) {
				hdrTexture.loadHDRMap("Images/HDRI/thatch_chapel_2k.hdr");
				updateIBL();
			}
			if (RadioButton("Thatch Chapel 4K", &currentSkybox, 4)) {
				hdrTexture.loadHDRMap("Images/HDRI/thatch_chapel_4k.hdr");
				updateIBL();
			}

			TreePop();
		}
		if (TreeNode("Post-processing")) {
			if (Checkbox("Using textures in sRGB color space?", &transformSRGB)) {
				PBRShader.use();
				PBRShader.set1b("transformSRGB", transformSRGB);

				shader.use();
				shader.set1b("transformSRGB", transformSRGB);
			}
			NewLine();

			Text("Anti-Aliasing");
			if (RadioButton("Off", &antiAliasing, 0)) {
				postprocShader.use();
				postprocShader.set1b("fxaaEnabled", false);
			}
			if (RadioButton("MSAA", &antiAliasing, 1)) {
				postprocShader.use();
				postprocShader.set1b("fxaaEnabled", false);
			}
			if(RadioButton("FXAA", &antiAliasing, 2)) {
				postprocShader.use();
				postprocShader.set1b("fxaaEnabled", true);
			}
			
			BeginDisabled(antiAliasing != 1);
			if (SliderInt("MSAA Sample Count", &msaaSampleCount, 1, 8))
				setupMSAA();
			EndDisabled();
			NewLine();

			Text("FXAA Options");
			BeginDisabled(antiAliasing != 2);
			if (SliderFloat("FXAA_EDGE_THRESHOLD_MIN", &EDGE_THRESHOLD_MIN, 0, 1, "%.4f")) {
				postprocShader.use();
				postprocShader.set1f("EDGE_THRESHOLD_MIN", EDGE_THRESHOLD_MIN);
				fxaaRevert = true;
			}
			if (SliderFloat("FXAA_EDGE_THRESHOLD_MAX", &EDGE_THRESHOLD_MAX, 0, 1, "%.4f")) {
				postprocShader.use();
				postprocShader.set1f("EDGE_THRESHOLD_MAX", EDGE_THRESHOLD_MAX);
				fxaaRevert = true;
			}
			if (SliderInt("FXAA_ITERATIONS", &ITERATIONS, 0, 100)) {
				postprocShader.use();
				postprocShader.set1i("ITERATIONS", ITERATIONS);
				fxaaRevert = true;
			}
			if (SliderFloat("FXAA_SUBPIXEL_QUALITY", &SUBPIXEL_QUALITY, 0, 1, "%.4f")) {
				postprocShader.use();
				postprocShader.set1f("SUBPIXEL_QUALITY", SUBPIXEL_QUALITY);
				fxaaRevert = true;
			}
			if (fxaaRevert) {
				if(Button("Revert")) {
					EDGE_THRESHOLD_MIN = DEFAULT_EDGE_THRESHOLD_MIN;
					EDGE_THRESHOLD_MAX = DEFAULT_EDGE_THRESHOLD_MAX;
					ITERATIONS		   = DEFAULT_ITERATIONS;
					SUBPIXEL_QUALITY   = DEFAULT_SUBPIXEL_QUALITY;

					postprocShader.use();
					postprocShader.set1f("EDGE_THRESHOLD_MIN", EDGE_THRESHOLD_MIN);
					postprocShader.set1f("EDGE_THRESHOLD_MAX", EDGE_THRESHOLD_MAX);
					postprocShader.set1i("ITERATIONS", ITERATIONS);
					postprocShader.set1f("SUBPIXEL_QUALITY", SUBPIXEL_QUALITY);

					fxaaRevert = false;
				}
			}
			EndDisabled();
			
			NewLine();

			if (TreeNode("Tonemapping")) {
				std::vector<const char*> tonemapModes = { "None", "Reinhard", "Extended Reinhard", "Uncharted2", "Hill ACES", "Narkowicz ACES", "Manual Exposure" };

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

			if (TreeNode("Gamma Correction")) {
				if (Checkbox("Use Gamma Correction", &gammaOn)) {
					postprocShader.use();
					postprocShader.set1b("gammaOn", gammaOn);
				}

				BeginDisabled(!gammaOn);
				if (SliderFloat("Gamma", &gamma, 0.f, 10.f)) {
					postprocShader.use();
					postprocShader.set1f("gamma", gamma);
				}

				EndDisabled();
				TreePop();
			}
			TreePop();
		}
	}
	if (CollapsingHeader("Shaders")) {
		if (Button("Reload")) {
			shader.loadShader("Shaders/main.vert", "Shaders/main.frag");

			shader.use();
			dirLight.set(shader, "dirLight");
			pointLight.set(shader, "pointLights[0]");
			spotLight.set(shader, "spotLight");

			shader.set1b("dirLightEnabled", dirLightEnabled);
			shader.set1b("pointLightEnabled", pointLightEnabled);
			shader.set1b("spotLightEnabled", spotLightEnabled);
			shader.setMat4("proj", proj);
			
			shader.set1b("transformSRGB", transformSRGB);
		}
		SameLine(); Text("Main Shader");

		if (Button("Reload##0")) {
			postprocShader.loadShader("Shaders/renderQuad.vert", "Shaders/postProc.frag");

			postprocShader.use();
			//postprocFlags.set(postprocShader, "flags");
			postprocShader.set1b("gammaOn", gammaOn);
			postprocShader.set1b("bloomOn", bloomOn);
			postprocShader.set1b("blurOn", blurOn);

			postprocShader.set1f("exposure", exposure);
			postprocShader.set1f("gamma", gamma);
			postprocShader.set1f("maxRadiance", maxRadiance);
			postprocShader.set1i("tonemapMode", tonemapMode);

			//FXAA
			postprocShader.set1b("fxaaEnabled", antiAliasing == 2);
			postprocShader.setVec2("inverseScreenSize", glm::vec2(1.f / SCR_WIDTH, 1.f / SCR_HEIGHT));
			postprocShader.set1f("EDGE_THRESHOLD_MIN", EDGE_THRESHOLD_MIN);
			postprocShader.set1f("EDGE_THRESHOLD_MAX", EDGE_THRESHOLD_MAX);
			postprocShader.set1i("ITERATIONS", ITERATIONS);
			postprocShader.set1f("SUBPIXEL_QUALITY", SUBPIXEL_QUALITY);
		}
		SameLine(); Text("Post-processing Shader");

		if (Button("Reload##1")) {
			PBRShader.loadShader("Shaders/PBR/PBR.vert", "Shaders/PBR/PBR.frag");

			PBRShader.use();
			PBRShader.setMat4("proj", proj);
			
			dirLight.set(PBRShader, "dirLight");
			pointLight.set(PBRShader, "pointLights[0]");
			spotLight.set(PBRShader, "spotLight");

			PBRShader.set1b("pointLightEnabled", pointLightEnabled);
			PBRShader.set1b("dirLightEnabled", dirLightEnabled);
			PBRShader.set1b("spotLightEnabled", spotLightEnabled);

			PBRShader.set1b("iblEnabled", iblEnabled);

			PBRShader.set1b("useAlbedo", useAlbedo);
			PBRShader.set1b("useNormalMap", useNormalMap);
			PBRShader.set1b("useMetallic", useMetallic);
			PBRShader.set1b("useRoughness", useRoughness);
			PBRShader.set1b("useAlbedo", useAmbientMap);

			PBRShader.set1b("transformSRGB", transformSRGB);
		}
		SameLine(); Text("PBR Shader");
	}
	if (CollapsingHeader("Profiling", ImGuiTreeNodeFlags_DefaultOpen)) {
		Text(("IMGUI Average framerate: " + std::to_string((int)io.Framerate) + " FPS").c_str());
		Text(("IMGUI Average frametime: " + std::to_string(1000 / io.Framerate) + " ms").c_str());
		Text(("Delta Time: " + std::to_string(dt.deltaTime * 1000) + " ms").c_str());
		NewLine();

		Text("Query Time");
		double gpuFrametime = (renderPass.result + guiPass.result + postprocPass.result) / 1000000.0;
		//Text(("Approx. CPU time: " + std::to_string(1000 / io.Framerate - gpuFrametime) + " ms").c_str());
		Text(("Approx. GPU time: " + std::to_string(gpuFrametime) + " ms").c_str());
		NewLine();

		Text(("Render Pass:		" + std::to_string(renderPass.result / 1000000.0) + "  ms").c_str());
		Text(("GUI Pass:		" + std::to_string(guiPass.result / 1000000.0) + "  ms").c_str());
		Text(("Post-Proc Pass:	" + std::to_string(postprocPass.result / 1000000.0) + "  ms").c_str());
	}
	if (CollapsingHeader("OpenGL Options")) {
		if (Checkbox("VSync", &vsyncOn))
			glfwSwapInterval(vsyncOn);
	}
	if (CollapsingHeader("UI Options")) {
		SliderFloat("BG Alpha", &bgAlpha, 0.f, 1.f);

		std::vector<const char*> styles = { "Classic", "Dark", "Light" };
		if (Combo("Style", &currentStyle, styles.data(), styles.size())) {
			if (currentStyle == 0) StyleColorsClassic();
			else if (currentStyle == 1) StyleColorsDark();
			else if (currentStyle == 2) StyleColorsLight();
		}
	}
	if (CollapsingHeader("Software & Hardware Info")) {
		Text(("OpenGL Version: " + openGLVersion).c_str());
		Text(("GLSL Version: " + glslVersion).c_str());
		Text(("Vendor: " + vendor).c_str());
		Text(("GPU Version: " + gpuVersion).c_str());
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
	//ImGui::ShowMetricsWindow();
	//ImGui::ShowDemoWindow();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(GetDrawData());
	
	guiPass.end();
}