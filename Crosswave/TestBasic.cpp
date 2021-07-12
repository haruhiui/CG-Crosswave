
#include <iostream>
#include <vector> 

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"
#include "Shader.h"
#include "Window.h"
#include "Object.h"

#include "Skybox.h"
#include "Cubemaps.h"

void frameBufferSizeCallback(GLFWwindow* glfwWindow, int width, int height);
void mouseCallback(GLFWwindow* glfwWindow, double xpos, double ypos);
void scrollCallback(GLFWwindow* glfwWindow, double xoffset, double yoffset);
void processInput(GLFWwindow* glfwWindow);

// self-defined Window 
Window window;
// camera 
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f));

int main() {

	window.setFrameBufferSizeCallback(frameBufferSizeCallback);
	window.setCursorPosCallback(mouseCallback);
	window.setScrollCallback(scrollCallback);
	window.setProcessInput(processInput);

	// ------------


	std::vector<std::string> faces {
		"resources/textures/skybox/default/right.jpg",
		"resources/textures/skybox/default/left.jpg",
		"resources/textures/skybox/default/top.jpg",
		"resources/textures/skybox/default/bottom.jpg",
		"resources/textures/skybox/default/front.jpg",
		"resources/textures/skybox/default/back.jpg"
	};
	
	Shader skyboxShader("skybox.vs", "skybox.fs"); 
	Shader cubemapsShader("cubemaps.vs", "cubemaps.fs"); 
	Shader cycleShader("cycle.vs", "cycle.fs"); 

	Skybox skybox(faces); 
	skybox.setShader(&skyboxShader); 
	skybox.bindVertexArray(); 
	skybox.bindTexture(); 

	Cubemaps cube(faces); 
	cube.setShader(&cubemapsShader); 
	cube.bindVertexArray(); 
	cube.bindTexture(); 

	// render loop
	while (!glfwWindowShouldClose(window.glfwWindow)) {
		// per-frame time logic
		float currentFrame = glfwGetTime();
		window.deltaTime = currentFrame - window.lastFrame;
		window.lastFrame = currentFrame;

		// input
		processInput(window.glfwWindow);

		// render 
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// ------------
		skybox.render(window, camera); 
		cube.render(window, camera); 

		cycleShader.use(); 
		
		renderCycle(); 

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(window.glfwWindow);
		glfwPollEvents();
	}

	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow* glfwWindow) {
	if (glfwGetKey(glfwWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(glfwWindow, true);

	if (glfwGetKey(glfwWindow, GLFW_KEY_W) == GLFW_PRESS)
		camera.processKeyboard(FORWARD, window.deltaTime);
	if (glfwGetKey(glfwWindow, GLFW_KEY_S) == GLFW_PRESS)
		camera.processKeyboard(BACKWARD, window.deltaTime);
	if (glfwGetKey(glfwWindow, GLFW_KEY_A) == GLFW_PRESS)
		camera.processKeyboard(LEFT, window.deltaTime);
	if (glfwGetKey(glfwWindow, GLFW_KEY_D) == GLFW_PRESS)
		camera.processKeyboard(RIGHT, window.deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void frameBufferSizeCallback(GLFWwindow* glfwWindow, int width, int height) {
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
void mouseCallback(GLFWwindow* glfwWindow, double xpos, double ypos) {
	if (window.firstMouse) {
		window.lastX = xpos;
		window.lastY = ypos;
		window.firstMouse = false;
	}

	float xoffset = xpos - window.lastX;
	float yoffset = window.lastY - ypos; // reversed since y-coordinates go from bottom to top
	window.lastX = xpos;
	window.lastY = ypos;

	camera.processMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scrollCallback(GLFWwindow* glfwWindow, double xoffset, double yoffset) {
	camera.processMouseScroll(yoffset);
}
