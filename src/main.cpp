
#include <glad/glad.h>
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

using namespace ImGui;

unsigned int SCR_WIDTH = 1200;
unsigned int SCR_HEIGHT = 800;

float fov = 45.f;

Camera cam(glm::vec3(0.f, 1.f, 3.f), 7.5f);

void beginPostProcess();
void endPostProcess();
void renderScene(Shader& shader);
void processInput(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void updateImGui();

bool mouseLocked = true;
ImGuiIO io;

glm::mat4 model(1.f);
glm::mat4 view(1.f);
glm::mat4 proj(1.f);

ClassicMesh cube, plane;//, quad;
RenderQuad quad;

glm::mat4 cube1Model(1.f);
glm::mat4 cube2Model(1.f);
glm::mat4 planeModel(1.f);

//Shaders
Shader shader;
Shader skyboxShader;
Shader hdrShader;
Shader deferredShader;
Shader postprocShader;
Shader blurShader;
Shader debugQuadShader;

unsigned int postprocFBO, postprocColorBuffer, postprocRBO;
uint16_t postprocFlags = PostProcFlags_none;
//HDR
float exposure = 2.2f;

//Gamma
float gamma = 2.2f;

//Bloom
unsigned int bloomFBO;
unsigned int colorBuffers[2];

unsigned int pingpongFBO[2];
unsigned int pingpongBuffers[2];

//GL OPTIONS
bool MSAA = true;
bool depthTest = true;
bool cullFace = true;
bool glGamma = true;
glm::vec4 clearColor = glm::vec4(0.f, 0.f, 0.f, 1.f);

//Rendering
	//Objects
	bool drawSkybox = false;
	Skybox skybox;
	
	bool cube1Enabled = true;
	glm::vec3 cube1Pos = glm::vec3(-1.f, 0.501f, -2.f);
	bool cube2Enabled = true;
	glm::vec3 cube2Pos = glm::vec3(0.f, 0.501f, 0.f);
	bool planeEnabled = true;
	glm::vec3 planePos = glm::vec3(0.f);
	
	//Lights
	bool dirLightEnabled = false;
	bool pointLightEnabled = true;
	bool spotLightEnabled = false;
	
	DirLight dirLight(glm::vec3(-1.f, -1.f, -1.f), glm::vec3(.1f), glm::vec3(1.f), glm::vec3(.3f));
	PointLight pointLight(glm::vec3(0.f, .5f, 1.f), glm::vec3(.05f), glm::vec3(1.f), glm::vec3(.3f), 1.f, .014f, .000007f);
	SpotLight spotLight(cam.getPos(), cam.camFront, glm::vec3(0.f), glm::vec3(1.f), glm::vec3(.3f), 1.f, .09f, .032f, glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(15.f)));
	
	//Deferred Shading
	bool deferredShadingEnabled = false;
	int deferredState = 4;

int main() {
	//Initiate every setting and library
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);

	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//Configure GL options
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //or GL_LINE
	glEnable(GL_DEPTH_TEST);

	//Face cull
	if(cullFace) glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	//MSAA
	if(MSAA) glEnable(GL_MULTISAMPLE);

	//Gamma correction
	if(glGamma) glEnable(GL_FRAMEBUFFER_SRGB);

	//Input setup
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);

	//Background Color
	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.y);

	//Shaders
	shader = Shader("Shaders/shader.vert", "Shaders/shader.frag");
	skyboxShader = Shader("Shaders/skybox.vert", "Shaders/skybox.frag");
	hdrShader = Shader("Shaders/renderQuad.vert", "Shaders/hdr.frag");
	deferredShader = Shader("Shaders/shader.vert", "Shaders/deferred.frag");
	postprocShader = Shader("Shaders/renderQuad.vert", "Shaders/postProc.frag");
	blurShader = Shader("Shaders/renderQuad.vert", "Shaders/blur.frag");
	debugQuadShader = Shader("Shaders/renderQuad.vert", "Shaders/renderQuad.frag");

	debugQuadShader.use();
	debugQuadShader.set1i("image", 0);

	shader.use();

	//Matrices
	//glm::mat4 model = glm::mat4(1.f);
	shader.setMat4("model", model);

	//glm::mat4 view = glm::mat4(1.f);
	shader.setMat4("view", view);

	//glm::mat4 proj = glm::perspective(glm::radians(fov), SCR_WIDTH / (float)SCR_HEIGHT, .1f, 100.f);
	proj = glm::perspective(glm::radians(fov), SCR_WIDTH / (float)SCR_HEIGHT, .1f, 100.f);
	shader.setMat4("proj", proj);

	//BASIC SCENE
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

	cube = ClassicMesh(cubeVertices, {}, { ClassicTexture("Images/marble.jpg"),  ClassicTexture("Images/marble.jpg", "texture_specular") });
	plane = ClassicMesh(planeVertices, {}, { ClassicTexture("Images/metal.png"), ClassicTexture("Images/White.png", "texture_specular") });
	quad = RenderQuad(quadVertices, {});

	planeModel = glm::scale(planeModel, glm::vec3(10.f));

	//Skybox
	std::vector<std::string> textureFaces = {
		"Images/skybox/right.jpg",
		"Images/skybox/left.jpg",
		"Images/skybox/top.jpg",
		"Images/skybox/bottom.jpg",
		"Images/skybox/front.jpg",
		"Images/skybox/back.jpg"
	};
	skybox = Skybox(textureFaces);


	//Lights
	dirLight.set(shader, "dirLight");
	pointLight.set(shader, "pointLights[0]");
	spotLight.set(shader, "spotLight");

	shader.set1b("dirLightEnabled", dirLightEnabled);
	shader.set1b("pointLightEnabled", pointLightEnabled);
	shader.set1b("spotLightEnabled", spotLightEnabled);

	//Materials
	shader.set1f("material.shininess", 2);

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
	/*Normal Mapping
	glm::mat4 quadModel(1.f);
	quadModel = glm::rotate(quadModel, glm::radians(180.f), glm::vec3(-1.f, 0.f, 0.f));
	ClassicMesh quad(quadVertices, {}, { Texture("Images/brickwall.jpg") });
	Texture normalMap("Images/brickwall_normal.jpg");

	Model cyborg("Objects/cyborg/cyborg.obj");
	ClassicMesh quad(quadVertices, {}, {Texture("Images/bricks2.jpg"), Texture("Images/bricks2_normal.jpg", "texture_normal"), Texture("Images/bricks2_disp.jpg", "texture_displacement")});

	bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
	bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
	bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
	*/
	/*Paralax Mapping
	shader.set1f("height_scale", .1f);
	*/
	/*HDR*/
	//std::vector<PointLight> pointLights = {
	//	PointLight(glm::vec3(0.f, 1.f, 0.f), glm::vec3(.05f), glm::vec3(.5f), glm::vec3(.3f)),
	//	PointLight(glm::vec3(0.f, 1.f, 5.f), glm::vec3(.05f), glm::vec3(2.f), glm::vec3(.3f)),
	//	PointLight(glm::vec3(0.f, 1.f, 10.f), glm::vec3(.05f), glm::vec3(20.f), glm::vec3(.3f)),
	//};
	//for (int i = 0; i < pointLights.size(); i++)
	//	pointLights[i].set(shader, "pointLights[" + std::to_string(i) + "]");

	//unsigned int hdrFBO;
	//glGenFramebuffers(1, &hdrFBO);
	//
	//unsigned int colorBuffer;
	//glGenTextures(1, &colorBuffer);
	//glBindTexture(GL_TEXTURE_2D, colorBuffer);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//
	//unsigned int hdrRBODepth;
	//glGenRenderbuffers(1, &hdrRBODepth);
	//glBindRenderbuffer(GL_RENDERBUFFER, hdrRBODepth);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
	//// attach buffers
	//glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, hdrRBODepth);
	//if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	//	std::cout << "Framebuffer not complete!" << std::endl;
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

	/*Bloom*/
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
	shader.set1b("bloomOn", postprocFlags & PostProcFlags_bloom);

	/*Deferred rendering*/
	unsigned int gBuffer;
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	unsigned int gPosition, gNormal, gAlbedoSpec;

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
	//Shader lightPassShader("Shaders/renderQuad.vert", "Shaders/lightpass.frag");

	//lightPassShader.use();
	//initLights(lightPassShader);
	//lightPassShader.set1f("material.shininess", 2.f);
	//
	//lightPassShader.set1i("positionBuffer", 0);
	//lightPassShader.set1i("normalBuffer", 1);
	//lightPassShader.set1i("albedoBuffer", 2);
	//
	//lightPassShader.set1i("deferredState", deferredState);
	shader.use();
	shader.set1i("positionBuffer", 0);
	shader.set1i("normalBuffer", 1);
	shader.set1i("albedoBuffer", 2);

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
	postprocShader.set1ui("flags", postprocFlags);

	postprocShader.set1f("exposure", exposure);
	postprocShader.set1i("hdrBuffer", 0);

	postprocShader.set1f("gamma", gamma);

	//IMGUI
	IMGUI_CHECKVERSION();
	CreateContext();
	//ImGuiIO& io = GetIO(); (void)io;
	//StyleColorsDark();
	StyleColorsClassic();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	while (!glfwWindowShouldClose(window)) {
		//Setup
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		processInput(window);
		cam.processInput(window);

		//Setup IMGUI
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		NewFrame();

		io = GetIO();

		cam.updateView();
		view = cam.getView();

		shader.use();

		shader.setVec3("viewPos", cam.getPos());
		shader.setMat4("proj", proj);
		shader.setMat4("view", view);

		if (spotLightEnabled) {
			shader.setVec3("spotLight.position", cam.getPos());
			shader.setVec3("spotLight.direction", cam.camFront);
		}

		shader.set1i("deferredState", deferredState);
		shader.set1b("renderQuad", false);
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
		/*Normal map test scene
		normalMap.bind(2);
		shader.setMat4("model", quadModel);
		shader.set1i("normalMap", 2);

		quad.Draw(shader);
		*/
		/*Bloom
		glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.setMat4("model", planeModel);
		plane.Draw(shader);

		shader.setMat4("model", cube1Model);
		cube.Draw(shader);

		shader.setMat4("model", cube2Model);
		cube.Draw(shader);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		bool horizontal = true, first_iteration = true;
		int amount = 10;


		blurShader.use();
		glDisable(GL_DEPTH_TEST);

		//glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[0]);
		//glClear(GL_COLOR_BUFFER_BIT);
		//glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[1]);
		//glClear(GL_COLOR_BUFFER_BIT);
		for (unsigned int i = 0; i < amount; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);

			blurShader.set1i("horizontal", horizontal);
			blurShader.set1i("image", 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(
				GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongBuffers[!horizontal]
			);

			quad.Draw(blurShader);

			horizontal = !horizontal;
			if (first_iteration)
				first_iteration = false;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		bloomShader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, pingpongBuffers[1]);

		bloomShader.set1i("scene", 0);
		bloomShader.set1i("bloomBlur", 1);
		bloomShader.set1f("exposure", 1.f);

		quad.Draw(bloomShader);
		*/
		/*Deferred Rendering*/

		if (deferredShadingEnabled) {
			glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
			glClearColor(0.0, 0.0, 0.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			deferredShader.use();

			deferredShader.setMat4("proj", proj);
			deferredShader.setMat4("view", view);

			renderScene(deferredShader);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			beginPostProcess();
			quad.Draw(shader, { gPosition, gNormal, gAlbedoSpec });
		}
		else {
			beginPostProcess();
			renderScene(shader);
		}
		endPostProcess();

		//Finish with imGui
		updateImGui();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	//End of program
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	DestroyContext();

	glfwTerminate();
	return 0;
}
void beginPostProcess() {
	if ((bool)(postprocFlags & PostProcFlags_bloom))
		glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO);
	else
		glBindFramebuffer(GL_FRAMEBUFFER, postprocFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void endPostProcess() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if ((bool)(postprocFlags & PostProcFlags_bloom)) {
		bool horizontal = true, first_iteration = true;
		int amount = 10;


		blurShader.use();

		for (unsigned int i = 0; i < amount; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);

			blurShader.set1i("horizontal", horizontal);

			quad.Draw(blurShader, { first_iteration ? colorBuffers[1] : pingpongBuffers[!horizontal] });

			horizontal = !horizontal;
			if (first_iteration)
				first_iteration = false;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		quad.Draw(postprocShader, { colorBuffers[0], pingpongBuffers[1] });
	}
	else
		quad.Draw(postprocShader, { postprocColorBuffer });
}
void renderScene(Shader& shader) {
	shader.use();

	shader.setMat4("model", planeModel);
	shader.setVec3("pos", planePos);
	plane.Draw(shader, planeEnabled);
	shader.setMat4("model", glm::mat4(1.f)); //Set the uniform to default

	//shader.setMat4("model", cube1Model);
	shader.setVec3("pos", cube1Pos);
	cube.Draw(shader, cube1Enabled);

	//shader.setMat4("model", cube2Model);
	shader.setVec3("pos", cube2Pos);
	cube.Draw(shader, cube2Enabled);

	//Draw Skybox last
	if (drawSkybox) skybox.Draw(skyboxShader, view, proj);
}
void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		mouseLocked = false;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS && !io.WantCaptureMouse) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		mouseLocked = true;
	}
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	SCR_WIDTH = width;
	SCR_HEIGHT = height;

	proj = glm::perspective(glm::radians(fov), SCR_WIDTH / (float)SCR_HEIGHT, .1f, 100.f);
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
	SetNextWindowSizeConstraints(ImVec2(200, SCR_HEIGHT), ImVec2(SCR_WIDTH, SCR_HEIGHT));

	Begin("Option Window", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);

	SetWindowPos(ImVec2(SCR_WIDTH - GetWindowWidth(), 0));

	if (CollapsingHeader("Rendering", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (TreeNode("Objects")) {
			Text("Skybox");
			Checkbox("Draw Skybox", &drawSkybox);

			Text("Cube1");
			Checkbox("Enabled##0", &cube1Enabled);
			BeginDisabled(!cube1Enabled);
			SliderFloat3("Pos##0", glm::value_ptr(cube1Pos), -10, 10);
			EndDisabled();

			Text("Cube2");
			Checkbox("Enabled##1", &cube2Enabled);
			BeginDisabled(!cube2Enabled);
			SliderFloat3("Pos##1", glm::value_ptr(cube2Pos), -10, 10);
			EndDisabled();

			Text("Plane");
			Checkbox("Enabled##2", &planeEnabled);
			BeginDisabled(!planeEnabled);
			SliderFloat3("Pos##2", glm::value_ptr(planePos), -10, 10);
			EndDisabled();

			//todo: implement light
			TreePop();
		}
		if (TreeNode("Lights")) {
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
			TreePop();
		}
		if (TreeNode("Deferred Shading/Rendering")) {
			Checkbox("Enable Deferred Shading", &deferredShadingEnabled);
			BeginDisabled(!deferredShadingEnabled);
			RadioButton("Display Position Buffer", &deferredState, 0);
			RadioButton("Display Normal Buffer", &deferredState, 1);
			RadioButton("Display Albedo Buffer", &deferredState, 2);
			RadioButton("Display Specular Buffer", &deferredState, 3);
			RadioButton("Display Combined", &deferredState, 4);
			EndDisabled();
			TreePop();
		}
	}
	if (CollapsingHeader("Post-processing", ImGuiTreeNodeFlags_DefaultOpen)) {
		//HDR, MSAA, blur, bloom, blur and such
		

		bool tempBool = postprocFlags & PostProcFlags_hdr;
		if (TreeNode("HDR")) {
			if (Checkbox("Use HDR", &tempBool)) {
				postprocShader.use();
				postprocFlags ^= PostProcFlags_hdr;
				postprocShader.set1ui("flags", postprocFlags);
			}
			BeginDisabled(!tempBool);

			if (SliderFloat("Exposure", &exposure, 0.f, 10.f)) {
				postprocShader.use();
				postprocShader.set1f("exposure", exposure);
			}

			EndDisabled();
			TreePop();
		}

		tempBool = postprocFlags & PostProcFlags_gammaCorrection;
		if (TreeNode("Gamma Correction")) {
			if (Checkbox("Use Gamma Correction", &tempBool)) {
				postprocShader.use();
				postprocFlags ^= PostProcFlags_gammaCorrection;
				postprocShader.set1ui("flags", postprocFlags);
			}

			BeginDisabled(!tempBool);
			if (SliderFloat("Gamma", &gamma, 0.f, 10.f)) {
				postprocShader.use();
				postprocShader.set1f("gamma", gamma);
				postprocShader.set1ui("flags", postprocFlags);
			}

			EndDisabled();
			TreePop();
		}

		tempBool = postprocFlags & PostProcFlags_bloom;
		if (Checkbox("Bloom", &tempBool)) {
			postprocShader.use();
			postprocFlags ^= PostProcFlags_bloom;
			postprocShader.set1ui("flags", postprocFlags);
			
			shader.use();
			shader.set1b("bloomOn", tempBool);
		}
		if(Checkbox("MSAA(doesn't work with deferred shading|NOT WORKING|)", &MSAA)){
			if (MSAA) glEnable(GL_MULTISAMPLE);
			else glDisable(GL_MULTISAMPLE);
		}
		//Todo:: implement other MSAA options, FXAA and such. Motion blur, blur, bloom and other.
	}
	if (CollapsingHeader("Options|NOT WORKING|")) {
	}
	if (CollapsingHeader("Profiling", ImGuiTreeNodeFlags_DefaultOpen)) {
		Text(("Average framerate: " + std::to_string((int)io.Framerate) + " FPS").c_str());
	}
	if (CollapsingHeader("About")) {
		unsigned int windowWidth = GetWindowWidth();
		TextWrapped("Render Engine made with OGL and C++ by Alexander Atanasov.", windowWidth);
		Spacing();
		Spacing();
		Spacing();
		TextWrapped("Email: pharosisdoingnothing@gmail.com", windowWidth);
		TextWrapped("LinkedIn: Alexander Atanasov", windowWidth);
		TextWrapped("Big thanks to Joey, the auther of Learn OpenGL.", windowWidth);
	}

	End();
	ImGui::ShowMetricsWindow();
	ImGui::ShowDemoWindow();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(GetDrawData());
}