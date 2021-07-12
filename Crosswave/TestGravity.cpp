
/* 2020.12.30 
 * 测试Object的重力、FigureObject的发射、NormalBullet的运动
 * 2020.12.31
 * 测试多个子弹的发射、测试碰撞
 * 2021.1.2
 * 添加右键、技能，限制相机位置y=0
 * 2021.1.5
 * 添加BloomBullet类
 */

#include <iostream>
#include <vector> 
#include <random> 
#include <cstdlib> 

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"
#include "Shader.h"
#include "Window.h"
#include "Object.h"

#include "Skybox.h"
#include "GeneralBullet.h"
#include "BulletObject.h"
#include "FigureObject.h"
#include "CubeRenderable.h" 
#include "CubeFigure.h"

void frameBufferSizeCallback(GLFWwindow* glfwWindow, int width, int height);
void mouseCallback(GLFWwindow* glfwWindow, double xpos, double ypos);
void scrollCallback(GLFWwindow* glfwWindow, double xoffset, double yoffset);
void mouseButtonCallback(GLFWwindow* glfwWindow, int button, int action, int mods); 
void processInput(GLFWwindow* glfwWindow);

// self-defined Window 
Window window;
bool mouseFocus = false;		// 鼠标捕获

// camera 
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f));

// shaders 
Shader skyboxShader("skybox.vs", "skybox.fs");
Shader generalBulletShader("general_bullet.vs", "general_bullet.fs");
Shader cubeShader("cube.vs", "cube.fs"); 

// 先用第一人称，之后再添加第三人称
FigureObject mainChar(Group::FRIEND, 100.0f);
std::list<CubeFigure*> enemies; 
std::list<GeneralBullet*> bullets;

int main() {

	window.setFrameBufferSizeCallback(frameBufferSizeCallback);
	window.setCursorPosCallback(mouseCallback);
	window.setScrollCallback(scrollCallback);
	window.setMouseButtonCallback(mouseButtonCallback); 
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

	Skybox skybox(faces);
	skybox.setShader(&skyboxShader);
	skybox.bindVertexArray();
	skybox.bindTexture();

	CubeFigure *enemy = new CubeFigure(Group::ENEMY, 10.0f);
	enemy->setShader(&cubeShader); 
	enemy->bindVertexArray();
	enemy->setCrashRadius(1.0f);

	enemies.push_back(enemy); 


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

		// 人物渲染
		for (std::list<CubeFigure*>::iterator iter = enemies.begin(); iter != enemies.end(); ) {
			(*iter)->render(window, camera); 
			
			// 人物与子弹碰撞判断
			for (std::list<GeneralBullet*>::iterator bltIter = bullets.begin(); bltIter != bullets.end(); ) {
				(*iter)->bulletCrash(*(*bltIter));	// bulletCrash自动检测碰撞

				if ((*bltIter)->isHit()) {
					GeneralBullet* temp = (*bltIter); 
					bltIter = bullets.erase(bltIter);
					delete temp; 
					std::cout << "erase bullet because of bullet hit" << std::endl; 
				}
				else {
					bltIter++; 
				}
			}

			// 人物存活判断
			if (!(*iter)->isAlive()) {
				CubeFigure* temp = (*iter); 
				iter = enemies.erase(iter); 
				delete temp; 
				std::cout << "erase figure because of bullet hit" << std::endl; 
			}
			else {
				iter++; 
			}
		}

		// 子弹渲染、出界判断
		for (std::list<GeneralBullet*>::iterator iter = bullets.begin(); iter != bullets.end(); ) {
			(*iter)->render(window, camera); 

			// 出界
			glm::vec3 pos = (*iter)->getPos().translateVec; 
			if (pos.x > 100.0f || pos.y > 200.0f || pos.z > 100.0f || 
				pos.x < -100.0f || pos.y < -10.0f || pos.z < -100.0f) {	// x: (-100, 100), y: (-20, 200), z: (-100, 100) 
				GeneralBullet* temp = (*iter); 
				iter = bullets.erase(iter);
				delete temp; 
				std::cout << "erase bullet because out of range" << std::endl; 
			}
			else {
				iter++; 
			}
		}

		// 如果没有敌人则随机生成
		if (enemies.size() == 0) {
			std::default_random_engine e;
			e.seed(time(NULL));				// 设置随机种子，否则每轮循环产生的都是一样的
			std::uniform_int_distribution<int> u(-50, 50);
			float x = u(e), y = 0, z = u(e);		// 随机获得新的敌人的xyz，xz范围(-20,20)，y=0
			std::cout << "new enemy generated: (" << x << ", " << y << ", " << z << ")" << std::endl; 

			CubeFigure* enemy = new CubeFigure(Group::ENEMY, 10.0f); 
			PositionProperty p = enemy->getPos(); 
			p.translateVec = glm::vec3(x, y, z); 
			enemy->setPos(p); 
			enemy->setShader(&cubeShader);
			enemy->bindVertexArray();
			enemies.push_back(enemy); 
		}

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(window.glfwWindow);
		glfwPollEvents();
	}

	return 0;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow* glfwWindow) {
	if (glfwGetKey(glfwWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		//glfwSetWindowShouldClose(glfwWindow, true);
		glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		// glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		mouseFocus = false;
		window.firstMouse = true; 
	}

	if (glfwGetKey(glfwWindow, GLFW_KEY_W) == GLFW_PRESS)
		camera.processKeyboard(FORWARD, window.deltaTime);
	if (glfwGetKey(glfwWindow, GLFW_KEY_S) == GLFW_PRESS)
		camera.processKeyboard(BACKWARD, window.deltaTime);
	if (glfwGetKey(glfwWindow, GLFW_KEY_A) == GLFW_PRESS)
		camera.processKeyboard(LEFT, window.deltaTime);
	if (glfwGetKey(glfwWindow, GLFW_KEY_D) == GLFW_PRESS)
		camera.processKeyboard(RIGHT, window.deltaTime);

	// skil 
	if (glfwGetKey(glfwWindow, GLFW_KEY_F) == GLFW_PRESS && mainChar.skillReady()) 
		for (int i = 0; i < mainChar.getSkillNum(); i++) {
			GeneralBullet* blt = new GeneralBullet(Group::FRIEND, 1.0f);
			blt->setShader(&generalBulletShader);
			blt->bindVertexArray();
			mainChar.skill(blt, camera.getFront());
			bullets.push_back(blt);
		}

	// 将相机位置一直限制在海面，即y=0处
	camera.limitHeight(); 

	mainChar.setPos(PositionProperty(camera.getPosition())); 
	glm::vec3 p = mainChar.getPos().translateVec; 
	// std::cout << p.x << " " << p.y << " " << p.z << std::endl; 
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void frameBufferSizeCallback(GLFWwindow* glfwWindow, int width, int height) {
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
void mouseCallback(GLFWwindow* glfwWindow, double xpos, double ypos) {
	if (mouseFocus) {
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
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scrollCallback(GLFWwindow* glfwWindow, double xoffset, double yoffset) {
	camera.processMouseScroll(yoffset);
}

// mouse button callback 
void mouseButtonCallback(GLFWwindow* glfwWindow, int button, int action, int mods) {
	if (action == GLFW_PRESS) {
		switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT: {
			if (!mouseFocus) {
				glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				mouseFocus = true; 
			}
			else {
				// shoot 
				GeneralBullet* blt = new GeneralBullet(Group::FRIEND, 1.0f);
				blt->setShader(&generalBulletShader);
				blt->bindVertexArray();
				mainChar.shoot(blt, camera.getFront());
				bullets.push_back(blt);
				break;
			}
		}
		case GLFW_MOUSE_BUTTON_MIDDLE: 
			break; 
		case GLFW_MOUSE_BUTTON_RIGHT:
			// right shoot 
			for (int i = 0; i < mainChar.getShootRightNum(); i++) {
				GeneralBullet* blt = new GeneralBullet(Group::FRIEND, 1.0f);
				blt->setShader(&generalBulletShader);
				blt->bindVertexArray();
				mainChar.shootRight(blt, camera.getFront());
				bullets.push_back(blt);
			}
			break; 
		}
	}
}