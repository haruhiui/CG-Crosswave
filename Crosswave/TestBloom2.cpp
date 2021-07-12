
/* 2020.12.30
 * 测试Object的重力、FigureObject的发射、NormalBullet的运动
 * 2020.12.31
 * 测试多个子弹的发射、测试碰撞
 * 2021.1.2
 * 添加右键、技能，限制相机位置y=0
 * 2021.1.5
 * 添加Bloom
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

#include "util.h" 
#include "Skybox.h"
#include "GeneralBullet.h"
#include "BulletObject.h"
#include "FigureObject.h"
#include "CubeRenderable.h" 
#include "CubeFigure.h"
#include "SimpleBullet.h"
#include "SimpleFigure.h"

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

Shader shader("7.bloom.vs", "7.bloom.fs");
Shader bloomLightShader("7.bloom.vs", "7.light_box.fs");
Shader bloomBlurShader("7.blur.vs", "7.blur.fs");
Shader bloomFinalShader("7.bloom_final.vs", "7.bloom_final.fs");

// 先用第一人称，之后再添加第三人称
FigureObject mainChar(Group::FRIEND, 100.0f);
// std::list<CubeFigure*> enemies;
std::list<SimpleFigure*> enemies; 
std::list<SimpleBullet*> bullets; 

const int fragLightNum = 40; 

int main() {

	window.setFrameBufferSizeCallback(frameBufferSizeCallback);
	window.setCursorPosCallback(mouseCallback);
	window.setScrollCallback(scrollCallback);
	window.setMouseButtonCallback(mouseButtonCallback);
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

	Skybox skybox(faces);
	skybox.setShader(&skyboxShader);
	skybox.bindVertexArray();
	skybox.bindTexture();

	// load textures
	// -------------
	unsigned int woodTexture = loadTexture("resources/textures/wood.png", true); // note that we're loading the texture as an SRGB texture
	unsigned int containerTexture = loadTexture("resources/textures/container2.png", true); // note that we're loading the texture as an SRGB texture

	// configure (floating point) framebuffers
	// ---------------------------------------
	unsigned int hdrFBO;
	glGenFramebuffers(1, &hdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	// create 2 floating point color buffers (1 for normal rendering, other for brightness threshold values)
	unsigned int colorBuffers[2];
	glGenTextures(2, colorBuffers);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, window.scrWidth, window.scrHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// attach texture to framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
	}
	// create and attach depth buffer (renderbuffer)
	unsigned int rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, window.scrWidth, window.scrHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);
	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// ping-pong-framebuffer for blurring
	unsigned int pingpongFBO[2];
	unsigned int pingpongColorbuffers[2];
	glGenFramebuffers(2, pingpongFBO);
	glGenTextures(2, pingpongColorbuffers);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, window.scrWidth, window.scrHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
		// also check if framebuffers are complete (no need for depth buffer)
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
	}    
	
	shader.use();
	shader.setInt("diffuseTexture", 0);
	bloomBlurShader.use();
	bloomBlurShader.setInt("image", 0);
	bloomFinalShader.use();
	bloomFinalShader.setInt("scene", 0);
	bloomFinalShader.setInt("bloomBlur", 1);

	// render loop
	while (!glfwWindowShouldClose(window.glfwWindow)) {
		// per-frame time logic
		float currentFrame = glfwGetTime();
		window.deltaTime = currentFrame - window.lastFrame;
		window.lastFrame = currentFrame;

		static bool readyPrinted = false; 
		if (!readyPrinted && mainChar.isSkillReady()) {		// 如果技能准备好且未提示过则输出提示
			std::cout << "skill ready" << std::endl;
			readyPrinted = true; 
		}
		if (readyPrinted && !mainChar.isSkillReady()) {		// 如果没准备号却提示过则设置还未提示
			readyPrinted = false; 
		}
			

		// input
		processInput(window.glfwWindow);

		// 判断人物与子弹碰撞
		for (auto iter = enemies.begin(); iter != enemies.end(); ) {
			for (auto bltIter = bullets.begin(); bltIter != bullets.end(); bltIter++) {
				(*iter)->hitOrMiss((*bltIter)); 
			}

			// 判断hp是否清零，清零则注销
			if ((*iter)->hp <= 0.0f) {
				SimpleFigure* temp = (*iter); 
				iter = enemies.erase(iter); 
				delete temp; 
				std::cout << "erase figure because hp = 0" << std::endl; 
				continue; 
			}
			iter++; 
		}

		// 如果人物全部消失则随机产生
		if (enemies.size() == 0) {
			std::default_random_engine e;
			e.seed(time(NULL));				// 设置随机种子，否则每轮循环产生的都是一样的
			std::uniform_int_distribution<int> u(-50, 50);
			float x = u(e), y = 0, z = u(e);		// 随机获得新的敌人的xyz，xz范围(-20,20)，y=0
			std::cout << "new enemy generated: (" << x << ", " << y << ", " << z << ")" << std::endl;

			SimpleFigure* enemy = new SimpleFigure(); 
			enemy->setFigureProp(Group::ENEMY, 10.0f); 
			enemy->setMotionProp(glm::vec3(0.0f), glm::vec3(0.0f)); 
			enemy->setPosProp(glm::vec3(x, y, z), glm::vec3(1.0f)); 
			enemies.push_back(enemy);
		}

		// 判断子弹出界、命中，根据情况注销子弹
		for (auto iter = bullets.begin(); iter != bullets.end(); ) {		// 注意这里不能在for循环的最后加iter++
			// 出界
			glm::vec3 pos = (*iter)->translate;
			if (pos.x > 100.0f || pos.y > 200.0f || pos.z > 100.0f ||
				pos.x < -100.0f || pos.y < -10.0f || pos.z < -100.0f) {	// x: (-100, 100), y: (-20, 200), z: (-100, 100) 
				SimpleBullet* temp = (*iter);
				iter = bullets.erase(iter);
				delete temp;
				std::cout << "erase bullet because out of range" << std::endl;
				continue;
			}
			// 击中
			if ((*iter)->hit) {
				SimpleBullet* temp = (*iter);
				iter = bullets.erase(iter);
				delete temp;
				std::cout << "erase bullet because hit" << std::endl;
				continue;
			}
			iter++;
		}

		// render
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 1. render scene into floating point framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		skybox.render(window, camera);

		glm::mat4 projection = glm::perspective(glm::radians(camera.getZoom()), (float)window.scrWidth / (float)window.scrHeight, 0.1f, 100.0f);
		glm::mat4 view = camera.getViewMatrix();
		glm::mat4 model = glm::mat4(1.0f);
		shader.use();
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, woodTexture);

		// set lighting uniforms
		int i = 0; 
		for (auto iter = bullets.begin(); i < fragLightNum && iter != bullets.end(); iter++) {
			shader.setVec3("lights[" + std::to_string(i) + "].Position", (*iter)->translate); 
			shader.setVec3("lights[" + std::to_string(i) + "].Color", (*iter)->color); 
			i++; 
		}
		while (i < fragLightNum) {
			shader.setVec3("lights[" + std::to_string(i) + "].Color", glm::vec3(0.0f));			// 将后面的颜色全部设置为0，防止产生已注销子弹仍然有在此处留有颜色的效果
			i++; 
		}
		shader.setVec3("viewPos", camera.getPosition());

		// create one large cube that acts as the floor
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0));
		model = glm::scale(model, glm::vec3(12.5f, 0.5f, 12.5f));
		shader.setMat4("model", model);
		renderCube();

		// 用shader渲染几个场景物体
		glBindTexture(GL_TEXTURE_2D, containerTexture);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 2.7f, 4.0));
		model = glm::rotate(model, glm::radians(23.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
		model = glm::scale(model, glm::vec3(1.25));
		shader.setMat4("model", model);
		renderCube();

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-2.0f, 1.0f, -3.0));
		model = glm::rotate(model, glm::radians(124.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
		shader.setMat4("model", model);
		renderCube(); 

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-1.0f, -1.0f, 2.0));
		model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
		shader.setMat4("model", model);
		renderCube();

		// 用shader渲染敌人
		for (auto iter = enemies.begin(); iter != enemies.end(); iter++) {
			model = glm::mat4(1.0f);
			model = glm::translate(model, (*iter)->translate); 
			model = glm::scale(model, (*iter)->scale); 
			shader.setMat4("model", model); 
			renderCube(); 
			(*iter)->update(); 
		}

		// 用bloomLightShader渲染子弹
		bloomLightShader.use();
		bloomLightShader.setMat4("projection", projection);
		bloomLightShader.setMat4("view", view);
		for (auto iter = bullets.begin(); iter != bullets.end(); iter++) { 
			model = glm::mat4(1.0f);
			model = glm::translate(model, (*iter)->translate);
			model = glm::scale(model, (*iter)->scale);
			bloomLightShader.setMat4("model", model);
			bloomLightShader.setVec3("lightColor", (*iter)->color);
			renderSphere();
			(*iter)->update(); 
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 2. blur bright fragments with two-pass Gaussian Blur 
		bool horizontal = true, first_iteration = true; 
		unsigned int amount = 20;
		bloomBlurShader.use();
		for (unsigned int i = 0; i < amount; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
			bloomBlurShader.setInt("horizontal", horizontal);
			glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
			renderQuad();
			horizontal = !horizontal;
			if (first_iteration)
				first_iteration = false;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		bool bloom = true; 
		float exposure = 1.0f; 

		// 3. now render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		bloomFinalShader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
		bloomFinalShader.setInt("bloom", bloom);
		bloomFinalShader.setFloat("exposure", exposure);
		renderQuad();

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

	// skill
	if (glfwGetKey(glfwWindow, GLFW_KEY_F) == GLFW_PRESS && mainChar.isSkillReady())
		for (int i = 0; i < mainChar.getSkillNum(); i++) {
			SimpleBullet* blt = new SimpleBullet(); 
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
				SimpleBullet *blt = new SimpleBullet(); 
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
				SimpleBullet* blt = new SimpleBullet(); 
				mainChar.shootRight(blt, camera.getFront());
				bullets.push_back(blt);
			}
			break;
		}
	}
}