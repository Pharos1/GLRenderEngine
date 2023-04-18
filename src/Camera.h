#pragma once
#ifndef CAMERA
#define CAMERA

#include <GLFW/glfw3.h>

#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>


#include "DeltaTime.h"
//Je

void mouse_callback(GLFWwindow* window, double xpos, double ypos);

class Camera {
private:
	GLFWwindow* window;

	glm::mat4 view;


	float pitch = 0.f;
	float yaw = -90.f;

	float camSpeed = 7.5f;

	bool firstMouse = true;

public:
	double lastX, lastY;
	glm::vec3 camPos = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 camFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);

	Camera(glm::vec3 camPos, float camSpeed = 7.5f) {
		this->camPos = camPos;
		this->camSpeed = camSpeed;
	}
	glm::mat4 getView() { return view; }
	glm::vec3 getPos()  { return camPos; }

	void updateView() { view = glm::lookAt(camPos, camPos + camFront, camUp); }
	void setCamSpeed(float camSpeed = 7.5f) { this->camSpeed = camSpeed; }

	void processInput(GLFWwindow* window) {
		float deltaTime = dt.getDT();
		float speedAmplifier = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? 3.f : (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ? 0.15 : 1.f);


		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camPos += camSpeed* camFront* deltaTime* speedAmplifier;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camPos -= camSpeed * camFront * deltaTime * speedAmplifier;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camPos -= glm::normalize(glm::cross(camFront, camUp)) * camSpeed * deltaTime * speedAmplifier;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camPos += glm::normalize(glm::cross(camFront, camUp)) * camSpeed * deltaTime * speedAmplifier;
	}
	void processMouse(double xPos, double yPos) {
		if (firstMouse)
		{
			lastX = xPos;
			lastY = yPos;
			firstMouse = false;
		}

		float xOffset = xPos - lastX;
		float yOffset = lastY - yPos;
		lastX = xPos;
		lastY = yPos;

		float sensitivity = 0.1f;
		xOffset *= sensitivity;
		yOffset *= sensitivity;

		yaw += xOffset;
		pitch += yOffset;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		glm::vec3 dir;
		dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		dir.y = sin(glm::radians(pitch));
		dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		camFront = glm::normalize(dir);
	}
};

#endif