

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
#include "Model.h" 

#include "util.h" 
#include "Skybox.h"
#include "SimpleBullet.h"
#include "SimpleFigure.h"
#include "Manjuu.h"

void frameBufferSizeCallback(GLFWwindow* glfwWindow, int width, int height);
void mouseCallback(GLFWwindow* glfwWindow, double xpos, double ypos);
void scrollCallback(GLFWwindow* glfwWindow, double xoffset, double yoffset);
void mouseButtonCallback(GLFWwindow* glfwWindow, int button, int action, int mods);
void processInput(GLFWwindow* glfwWindow);

glm::vec3 getEnemyShootUp(Manjuu* enemy); 
glm::vec3 getMainCharShootUp(Manjuu* enemy); 

// self-defined Window 
Window window;
bool mouseFocus = false;		// ��겶��

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
Shader cycleShader("cycle.vs", "cycle.fs");

Manjuu mainChar;
std::list<Manjuu*> enemies;
std::list<SimpleBullet*> bullets;

const int fragLightNum = 30;
const glm::vec3 shootUp = glm::vec3(0.0f, 0.3f, 0.0f);

// ��Ϸ����
int gameRound = 0; 

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
	unsigned int waterTexture = loadTexture("resources/textures/water.bmp", true); 

	// configure (floating point) framebuffers
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

	// tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
	stbi_set_flip_vertically_on_load(true);
	glEnable(GL_DEPTH_TEST);
	Shader modelShader("model.vs", "model.fs");
	Model modelModel("resources/models/AzurLane Manjuu/AzurLane Manjuu.obj");

	mainChar.setPosProp(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.01f), camera.getFront());
	mainChar.setMotionProp(glm::vec3(0.0f), glm::vec3(0.0f));
	mainChar.setFigureProp(Group::FRIEND, 10.0f, 1.0f);
	mainChar.hitRadius = 0.5f; 

	// render loop
	while (!glfwWindowShouldClose(window.glfwWindow)) {
		// per-frame time logic
		float currentFrame = glfwGetTime();
		window.deltaTime = currentFrame - window.lastFrame;
		window.lastFrame = currentFrame;

		static bool readyPrinted = false;
		if (!readyPrinted && mainChar.isSkillReady()) {		// �������׼������δ��ʾ���������ʾ
			std::cout << "skill ready" << std::endl;
			readyPrinted = true;
		}
		if (readyPrinted && !mainChar.isSkillReady()) {		// ���û׼����ȴ��ʾ�������û�δ��ʾ
			readyPrinted = false;
		}

		glClearColor(0.0f, 0.3f, 0.6f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// input
		processInput(window.glfwWindow);

		// �ж��������ӵ���ײ
		for (auto iter = enemies.begin(); iter != enemies.end(); ) {
			for (auto bltIter = bullets.begin(); bltIter != bullets.end(); bltIter++) {
				(*iter)->hitOrMiss((*bltIter));
			}

			// �ж�hp�Ƿ����㣬������ע��
			if ((*iter)->hp <= 0.0f) {
				Manjuu* temp = (*iter);
				iter = enemies.erase(iter);
				delete temp;
				// std::cout << "erase figure because hp = 0" << std::endl;
				continue;
			}
			iter++;
		}
		for (auto bltIter = bullets.begin(); bltIter != bullets.end(); bltIter++) {
			mainChar.hitOrMiss((*bltIter)); 
		}

		// ���Ƶ��˵����
		for (auto iter = enemies.begin(); iter != enemies.end(); iter++) {
			if ((*iter)->isShootReady()) {
				// std::cout << "enemy shoot" << std::endl; 
				for (int i = 0; i < (*iter)->shootNum; i++) {
					SimpleBullet* blt = new SimpleBullet();
					(*iter)->shoot(blt, mainChar.translate - (*iter)->translate);
					
					blt->speed += getEnemyShootUp((*iter)); 
					bullets.push_back(blt);
				}
			}
			//if ((*iter)->isShootRightReady()) {
			//	std::cout << "enemy shoot right" << std::endl;
			//	for (int i = 0; i < (*iter)->shootRightNum; i++) {
			//		SimpleBullet* blt = new SimpleBullet();
			//		(*iter)->shootRight(blt, mainChar.translate - (*iter)->translate);
			//		bullets.push_back(blt);
			//	}
			//}
			if ((*iter)->isSkillReady()) {
				// std::cout << "enemy skill" << std::endl;
				for (int i = 0; i < (*iter)->skillNum; i++) {
					SimpleBullet* blt = new SimpleBullet();
					(*iter)->skill(blt, mainChar.translate - (*iter)->translate);
					blt->speed += getEnemyShootUp((*iter));
					bullets.push_back(blt);
				}
			}
		}

		// �������ȫ����ʧ���������
		if (enemies.size() == 0) {
			std::default_random_engine e;
			e.seed(time(NULL));				// ����������ӣ�����ÿ��ѭ�������Ķ���һ����
			std::uniform_int_distribution<int> u(-50, 50);
			float x = u(e), y = 0.0f, z = u(e);		// �������µĵ��˵�xyz��xz��Χ(-20,20)��y=0
			// std::cout << "new enemy generated: (" << x << ", " << y << ", " << z << ")" << std::endl;

			int enemiesNum = (u(e) / 100 + 0.5) * gameRound;
			std::cout << "new enemies: " << enemiesNum << std::endl; 
			for (int i = 0; i < enemiesNum; i++) {
				Manjuu* enemy = new Manjuu();
				enemy->shootColor = glm::vec3(u(e) / 10 + 15, 3.0f, 3.0f);			// 10-20
				enemy->shootRightColor = glm::vec3(0.0f, 0.0f, u(e) / 10 + 15);
				enemy->skillColor = glm::vec3(25.0f, 1.0f, 0.0f);

				enemy->shootIntervalMax = u(e) / 20 + 5;		// 0-5
				enemy->skillInterval = u(e) / 25 + 5;

				enemy->setFigureProp(Group::ENEMY, 10.0f, 1.0f);
				enemy->setMotionProp(glm::vec3(0.0f), glm::vec3(0.0f));
				enemy->setPosProp(glm::vec3(u(e), 0.0f, u(e)), glm::vec3(0.03f), glm::vec3(1.0f, 0.0f, 1.0f));
				enemies.push_back(enemy);
			}
			gameRound++; 
		}

		// �ж��ӵ����硢���У��������ע���ӵ�
		for (auto iter = bullets.begin(); iter != bullets.end(); ) {		// ע�����ﲻ����forѭ��������iter++
			// ����
			glm::vec3 pos = (*iter)->translate;
			if (pos.x > 100.0f || pos.y > 200.0f || pos.z > 100.0f ||
				pos.x < -100.0f || pos.y < -10.0f || pos.z < -100.0f) {	// x: (-100, 100), y: (-20, 200), z: (-100, 100) 
				SimpleBullet* temp = (*iter);
				iter = bullets.erase(iter);
				delete temp;
				// std::cout << "erase bullet because out of range" << std::endl;
				continue;
			}
			// ����
			if ((*iter)->hit) {
				SimpleBullet* temp = (*iter);
				iter = bullets.erase(iter);
				delete temp;
				// std::cout << "erase bullet because hit" << std::endl;
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

		cycleShader.use();
		renderCycle();

		skybox.render(window, camera);

		glm::mat4 projection = glm::perspective(glm::radians(camera.getZoom()), (float)window.scrWidth / (float)window.scrHeight, 0.1f, 100.0f);
		glm::mat4 view = camera.getViewMatrix();
		glm::mat4 model = glm::mat4(1.0f);

		shader.use();
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);
		glActiveTexture(GL_TEXTURE0);
		// glBindTexture(GL_TEXTURE_2D, woodTexture);
		glBindTexture(GL_TEXTURE_2D, waterTexture); 

		// set lighting uniforms
		int i = 0;
		for (auto iter = bullets.rbegin(); i < fragLightNum && iter != bullets.rend(); iter++) {
			shader.setVec3("lights[" + std::to_string(i) + "].Position", (*iter)->translate);
			shader.setVec3("lights[" + std::to_string(i) + "].Color", (*iter)->color);
			i++;
		}
		while (i < fragLightNum) {
			shader.setVec3("lights[" + std::to_string(i) + "].Color", glm::vec3(0.0f));			// ���������ɫȫ������Ϊ0����ֹ������ע���ӵ���Ȼ���ڴ˴�������ɫ��Ч��
			i++;
		}
		shader.setVec3("viewPos", camera.getPosition());

		// create one large cube that acts as the floor
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0));
		model = glm::scale(model, glm::vec3(50.0f, 0.5f, 50.0f));
		shader.setMat4("model", model);
		renderCube();

		// ��shader��Ⱦ������������
		glBindTexture(GL_TEXTURE_2D, containerTexture);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(6.0f, 0.5f, 1.0));
		model = glm::scale(model, glm::vec3(0.5f));
		shader.setMat4("model", model);
		renderCube();

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
		model = glm::rotate(model, glm::radians(30.0f), glm::normalize(glm::vec3(-1.0, 1.0, 1.0)));
		model = glm::scale(model, glm::vec3(0.5f));
		shader.setMat4("model", model);
		renderCube();

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-4.0f, -1.0f, 2.0));
		model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
		shader.setMat4("model", model);
		renderCube();


		// ��shader��Ⱦ�Լ�
		model = mainChar.getModelMat();
		shader.setMat4("model", model);
		modelModel.draw(shader);
		mainChar.update();
		// �Լ��Ͳ���update�ˣ���processInput���ֶ�����

		// ��shader��Ⱦ����
		for (auto iter = enemies.begin(); iter != enemies.end(); iter++) {
			model = (*iter)->getModelMat();
			shader.setMat4("model", model);
			modelModel.draw(shader);
			(*iter)->update();
		}

		// ��bloomLightShader��Ⱦ�ӵ�
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

	if (glfwGetKey(glfwWindow, GLFW_KEY_W) == GLFW_PRESS) {
		mainChar.processKeyboard(FORWARD, window.deltaTime);
		// camera.processKeyboard(FORWARD, window.deltaTime);
	}
	if (glfwGetKey(glfwWindow, GLFW_KEY_S) == GLFW_PRESS) {
		mainChar.processKeyboard(BACKWARD, window.deltaTime);
	}
	if (glfwGetKey(glfwWindow, GLFW_KEY_A) == GLFW_PRESS) {
		mainChar.processKeyboard(LEFT, window.deltaTime);
	}
	if (glfwGetKey(glfwWindow, GLFW_KEY_D) == GLFW_PRESS) {
		mainChar.processKeyboard(RIGHT, window.deltaTime);
	}

	// �ֶ������Լ���ģ��
	glm::vec3 cameraFront = camera.getFront();
	cameraFront.y = 0;
	if (cameraFront == glm::vec3(0.0f))		cameraFront = glm::vec3(0.0f, 1.0f, 0.0f);
	else									cameraFront = glm::normalize(cameraFront);
	camera.setPosition(mainChar.translate - cameraFront * 5.0f + glm::vec3(0.0f, 0.5f, 0.0f));
	mainChar.front = camera.getFront();

	// skill
	if (glfwGetKey(glfwWindow, GLFW_KEY_F) == GLFW_PRESS && mainChar.isSkillReady())
		for (int i = 0; i < mainChar.skillNum; i++) {
			SimpleBullet* blt = new SimpleBullet();
			mainChar.skill(blt, camera.getFront() + shootUp);
			bullets.push_back(blt);
		}

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
		glm::vec3 cameraFront = camera.getFront();
		cameraFront.y = 0;
		if (cameraFront == glm::vec3(0.0f))		cameraFront = glm::vec3(0.0f, 1.0f, 0.0f);
		else									cameraFront = glm::normalize(cameraFront);
		camera.setPosition(mainChar.translate - cameraFront * 5.0f + glm::vec3(0.0f, 0.5f, 0.0f));
		mainChar.front = camera.getFront();

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
				SimpleBullet* blt = new SimpleBullet(); 
				
				bool aimed = false; 
				for (auto iter = enemies.begin(); iter != enemies.end(); iter++) {
					if (isAimed(camera.getFront(), (*iter)->translate - mainChar.translate, 0.99)) {
						mainChar.shoot(blt, (*iter)->translate - mainChar.translate); 
						blt->speed += getMainCharShootUp((*iter)); 
						aimed = true; 
						break; 
					}
				}
				if (!aimed) {
					mainChar.shoot(blt, camera.getFront() + shootUp);
				}

				bullets.push_back(blt);
				break;
			}
		}
		case GLFW_MOUSE_BUTTON_MIDDLE:
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			// right shoot 
			for (int i = 0; i < mainChar.shootRightNum; i++) {
				SimpleBullet* blt = new SimpleBullet();
				mainChar.shootRight(blt, camera.getFront() + shootUp);
				bullets.push_back(blt);
			}
			break;
		}
	}
}

glm::vec3 getEnemyShootUp(Manjuu* enemy) {
	glm::vec3 dist = mainChar.translate - enemy->translate; 
	float x = sqrt(glm::dot(dist, dist)); 
	float time = x / enemy->bulletSpeed; 
	glm::vec3 shootUp = glm::vec3(0.0f, 1.0f, 0.0f) * sqrt(glm::dot(GRAVITY_ACC, GRAVITY_ACC)) * (time / 2);
	return shootUp; 
}

glm::vec3 getMainCharShootUp(Manjuu* enemy) {
	glm::vec3 dist = mainChar.translate - enemy->translate;
	float x = sqrt(glm::dot(dist, dist));
	float time = x / mainChar.bulletSpeed;
	glm::vec3 shootUp = glm::vec3(0.0f, 1.0f, 0.0f) * sqrt(glm::dot(GRAVITY_ACC, GRAVITY_ACC)) * (time / 2);
	return shootUp;
}

