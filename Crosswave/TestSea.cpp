#define _CRT_SECURE_NO_WARNINGS      
#define FREEGLUT_STATIC

#include <stdio.h>
#include <string.h>      
#include <time.h>    
#include <stdlib.h>    
#include "fluid.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <math.h>
#include <iostream>

#include "Window.h"
#include "Camera.h"
#include "Shader.h"
#include "Cubemaps.h"
#include "Skybox.h"

using namespace std;

#ifndef GL_Lib
#ifdef _DEBUG
#define GL_Lib(name) name "d.lib"
#else
#define GL_Lib(name) name ".lib"
#endif
#endif
#pragma comment(lib, GL_Lib("freeglut_static"))


void frameBufferSizeCallback(GLFWwindow* glfwWindow, int width, int height);
void mouseCallback(GLFWwindow* glfwWindow, double xpos, double ypos);
void scrollCallback(GLFWwindow* glfwWindow, double xoffset, double yoffset);
void processInput(GLFWwindow* glfwWindow);

// self-defined Window 
Window window;
// camera 
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f));




//纹理缓冲区  
GLuint texture[13];

//视区      
float whRatio;
int wHeight = 0;
int wWidth = 0;

//流体对象
Fluid* f;

//视点      
float center[] = { 27, 10, 0 };
float eye[] = { 27,-50, 50 };

void loadTex(int i, const char* filename, GLuint* texture); 


void drawScene()
{
	static int count = 0;
	count++;
	if (count > 100) {
		count = 0;
		f->Evaluate();
	}
	f->draw();
}

//void updateView(int height, int width)
//{
//	glViewport(0, 0, width, height);
//	glMatrixMode(GL_PROJECTION);//设置矩阵模式为投影       
//	glLoadIdentity();   //初始化矩阵为单位矩阵          
//	whRatio = (GLfloat)width / (GLfloat)height;  //设置显示比例     
//	gluPerspective(45.0f, whRatio, 1.0f, 150.0f); //透视投影             
//	glMatrixMode(GL_MODELVIEW);  //设置矩阵模式为模型    
//}

//void reshape(int width, int height)
//{
//	if (height == 0)      //如果高度为0          
//	{
//		height = 1;   //让高度为1（避免出现分母为0的现象）          
//	}
//
//	wHeight = height;
//	wWidth = width;
//
//	updateView(wHeight, wWidth); //更新视角          
//}


//void idle()
//{
//	// glutPostRedisplay();
//}


void init()
{
	srand(unsigned(time(NULL)));
	glEnable(GL_DEPTH_TEST);//开启深度测试       

	glEnable(GL_LIGHTING);  //开启光照模式       

	glGenTextures(1, texture);
	loadTex(0, "water.bmp", texture);
	f = new Fluid(30, 30, 2, 1, 0.2, 0, texture[0]);
}

void redraw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//清除颜色和深度缓存        
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();   //初始化矩阵为单位矩阵          
	// gluLookAt(eye[0], eye[1], eye[2], center[0], center[1], center[2], 0, 1, 0);                // 场景（0，0，0)，Y轴向上      
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glShadeModel(GL_SMOOTH);
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	// 启用光照计算    
	glEnable(GL_LIGHTING);
	// 指定环境光强度（RGBA）    
	GLfloat ambientLight[] = { 2.0f, 2.0f, 2.0f, 1.0f };

	// 设置光照模型，将ambientLight所指定的RGBA强度值应用到环境光    
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);
	// 启用颜色追踪    
	glEnable(GL_COLOR_MATERIAL);
	// 设置多边形正面的环境光和散射光材料属性，追踪glColor    
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	drawScene();//绘制场景       
}

int main(int argc, char* argv[])
{

	window.setFrameBufferSizeCallback(frameBufferSizeCallback);
	window.setCursorPosCallback(mouseCallback);
	window.setScrollCallback(scrollCallback);
	window.setProcessInput(processInput);

	// ------------


	std::vector<std::string> faces{
		"resources/textures/skybox/default/right.jpg",
		"resources/textures/skybox/default/left.jpg",
		"resources/textures/skybox/default/top.jpg",
		"resources/textures/skybox/default/bottom.jpg",
		"resources/textures/skybox/default/front.jpg",
		"resources/textures/skybox/default/back.jpg"
	};

	Shader skyboxShader("skybox.vs", "skybox.fs");
	Shader cubemapsShader("cubemaps.vs", "cubemaps.fs");

	Skybox skybox(faces);
	skybox.setShader(&skyboxShader);
	skybox.bindVertexArray();
	skybox.bindTexture();

	Cubemaps cube(faces);
	cube.setShader(&cubemapsShader);
	cube.bindVertexArray();
	cube.bindTexture();

	init(); 

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

		redraw(); 

		// ------------
		skybox.render(window, camera);
		cube.render(window, camera);


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
