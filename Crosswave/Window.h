/* 
 * 参考自：https://learnopengl-cn.github.io/
 * 2020.12.30 添加鼠标点击回调
 */

#ifndef MYWINDOW_H
#define MYWINDOW_H 

#include <iostream>
#include <vector> 

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Window {

public: 
	GLFWwindow *glfwWindow; 

	// screen 
	//unsigned int scrWidth = 960; 
	//unsigned int scrHeight = 540; 

	unsigned int scrWidth = 1600;
	unsigned int scrHeight = 900;

	// mouse move 
	bool firstMouse = true; 
	float lastX = scrWidth / 2.0f; 
	float lastY = scrHeight / 2.0f; 

	// timing 
	float deltaTime = 0.0f; 
	float lastFrame = 0.0f; 

	// process input function 
	void(*processInput)(GLFWwindow *); 

public: 
	Window() {
		// glfw: initialize and configure
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

		// glfw window creation
		glfwWindow = glfwCreateWindow(scrWidth, scrHeight, "Crosswave", NULL, NULL);
		if (glfwWindow == NULL) {
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
			exit(-1);
		}
		// 
		glfwMakeContextCurrent(glfwWindow);
		// glfwSetWindowPos(glfwWindow, 500, 100); 
		glfwSetWindowPos(glfwWindow, 100, 100); 
		

		// tell GLFW to capture our mouse
		glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		// glad: load all OpenGL function pointers
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			std::cout << "Failed to initialize GLAD" << std::endl;
			exit(-1);
		}

		// configure global opengl state
		glEnable(GL_DEPTH_TEST);
	}

	~Window() {
		// glfw: terminate, clearing all previously allocated GLFW resources.
		glfwTerminate();
	}

	void setFrameBufferSizeCallback(GLFWframebuffersizefun frameBufferSizeCallback) const {
		glfwSetFramebufferSizeCallback(glfwWindow, frameBufferSizeCallback);
	}

	void setCursorPosCallback(GLFWcursorposfun mouseCallback) const {
		glfwSetCursorPosCallback(glfwWindow, mouseCallback);
	}

	void setScrollCallback(GLFWscrollfun scrollCallback) const {
		glfwSetScrollCallback(glfwWindow, scrollCallback);
	}

	void setMouseButtonCallback(GLFWmousebuttonfun mouseButtonCallback) const {
		glfwSetMouseButtonCallback(glfwWindow, mouseButtonCallback);
	}

	void setProcessInput(void(*processInput)(GLFWwindow *)) {
		this->processInput = processInput;
	}
};

#endif 