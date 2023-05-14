#pragma once
#ifndef DELTA_TIME
#define DELTA_TIME

#include <GLFW/glfw3.h>


struct DeltaTime {
	float deltaTime = 0.0f;	// Time between current frame and last frame
	float lastFrame = 0.0f; // Time of last frame

	void updateDT() {
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
	}
	float getDT() {
		updateDT();

		return deltaTime;
	}
};
DeltaTime dt;
#endif