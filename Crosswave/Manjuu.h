#pragma once

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
#include "SimpleBullet.h"


class Manjuu {

private:
	// time 
	float lastTime, deltaTime;

	const float yValue = 1.0f; 
	const float movementSpeed = 5.0f; 

public:
	// basic property 
	glm::vec3 translate, scale, front;

	// motion property 
	glm::vec3 speed, acceleration;

	// game property 
	Group group;
	float hp, maxHp;
	float hitRadius; 

	// shoot and skill 
	const float bulletSpeed = 20.0f;

	int shootNum = 1; 
	float shootDamage = 1.0f; 
	glm::vec3 shootColor = glm::vec3(15.0f, 7.0f, 0.0f); 
	glm::vec3 shootScale = glm::vec3(0.1f); 

	int shootRightNum = 3;
	float shootRightDamage = 1.0f;
	glm::vec3 shootRightColor = glm::vec3(0.0f, 7.0f, 15.0f); 
	glm::vec3 shootRightScale = glm::vec3(0.1f);

	int skillNum = 10;
	float skillDamage = 1.0f;
	glm::vec3 skillColor = glm::vec3(15.0f, 0.0f, 0.0f);
	glm::vec3 skillScale = glm::vec3(0.1f);
	// 控制释放间隔
	float shootIntervalMax = 8.0f;
	float shootInterval = 0.0f;
	float shootRightIntervalMax = 9.0f;
	float shootRightInterval = 0.0f;
	float skillIntervalMax = 10.0f; 
	float skillInterval = 0.0f;

public:
	Manjuu(glm::vec3 translate = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(0.03f), glm::vec3 front = glm::vec3(0.0f, -1.0f, 0.0f)) :
		translate(translate), scale(scale), front(front) {
		// 强迫front在xz平面上
		front.y = 0; 
		if (front == glm::vec3(0.0f)) {
			front = glm::vec3(0.0f, 0.0f, -1.0f); 
		}

		lastTime = glfwGetTime();
		deltaTime = 0;

		speed = glm::vec3(0.0f);
		acceleration = glm::vec3(0.0f);
		
		group = Group::FRIEND;
		hp = maxHp = 10.0f;
		hitRadius = 1.0f; 
	}
	
	~Manjuu() {

	}

	void setPosProp(glm::vec3 t, glm::vec3 s, glm::vec3 f) {
		translate = t;
		scale = s;
		front = f; 
		front.y = 0;
		if (front == glm::vec3(0.0f)) {
			front = glm::vec3(0.0f, 0.0f, -1.0f);
		}
	}

	void setMotionProp(glm::vec3 v, glm::vec3 a) {
		speed = v;
		acceleration = a;
	}

	void setFigureProp(Group group, float maxHp, float hitRadius) {
		this->group = group;
		this->maxHp = maxHp;
		this->hp = maxHp;
		this->hitRadius = hitRadius; 
	}

	glm::mat4 getModelMat() {
		glm::vec3 currFront(0.0f, 0.0f, -1.0f); 
		float deg = acos(glm::dot(currFront, glm::normalize(front))); 
		
		glm::mat4 model = glm::mat4(1.0f); 
		model = glm::translate(model, this->translate);
		model = glm::translate(model, glm::vec3(0.0f, -1.3f, 0.0f));
		// 在做旋转的时候要特别小心
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, -deg, glm::vec3(0.0f, 0.0f, 1.0f)); 
		model = glm::scale(model, this->scale);
		return model; 
	}

	// 自动判断子弹是否命中，命中则减去blt->damage的hp并设置blt->hit为true
	void hitOrMiss(SimpleBullet* blt) {
		glm::vec3 r = blt->translate - this->translate;
		float dist = glm::dot(r, r);
		r = blt->scale + this->hitRadius;
		float hitDist = glm::dot(r, r);

		if (dist < hitDist && blt->group != this->group) {
			hp -= blt->damage;
			blt->hit = true;

		}
	}

	// 射击
	bool isShootReady() {
		return shootInterval > shootIntervalMax; 
	}

	void shoot(SimpleBullet* blt, glm::vec3 dir) {
		// std::cout << "shoot" << std::endl;
		blt->setPosProp(translate, shootScale);
		blt->setMotionProp(bulletSpeed * glm::normalize(dir), GRAVITY_ACC);
		blt->setBulletProp(group, shootDamage); 
		blt->color = shootColor; 

		shootInterval = 0.0f; 
	}

	// y右键射击
	bool isShootRightReady() {
		return shootRightInterval > shootRightIntervalMax; 
	}

	void shootRight(SimpleBullet* blt, glm::vec3 dir) {
		static int cnt = -1;

		glm::vec3 verticalPos = glm::cross(dir, glm::vec3(0.0f, 1.0f, 0.0f));
		if (verticalPos == glm::vec3(0.0f, 0.0f, 0.0f)) {
			verticalPos = glm::vec3(1.0f, 0.0f, 0.0f);
		}
		verticalPos = glm::normalize(verticalPos);
		verticalPos.x *= cnt;
		verticalPos.y *= cnt;
		verticalPos.z *= cnt;

		// 将speed映射到xz平面，保持大小不变
		glm::vec3 speed = dir;
		speed.y = 0;
		if (speed == glm::vec3(0.0f, 0.0f, 0.0f)) {
			speed = glm::vec3(1.0f, 0.0f, 0.0f);
		}
		speed = glm::normalize(speed);

		if (cnt == 1)
			cnt = -1;
		else
			cnt++;

		// std::cout << "shoot right" << std::endl;
		blt->setPosProp(translate + verticalPos, shootRightScale);
		blt->translate.y = 0.0f;	// 设置在海面上
		blt->setMotionProp(bulletSpeed * speed, glm::vec3(0.0f));
		blt->setBulletProp(group, shootDamage);
		blt->color = shootRightColor;

		shootRightInterval = 0.0f; 
	}

	// 技能
	bool isSkillReady() {
		return skillInterval > skillIntervalMax; 
	}

	void skill(SimpleBullet* blt, glm::vec3 dir) {
		// 朝着speed方向发射十个子弹，方向添加随机部分
		static std::default_random_engine e(time(NULL));
		// e.seed(time(NULL));				// 设置随机种子，否则每轮循环产生的都是一样的
		std::uniform_int_distribution<int> u(-100, 100);

		glm::vec3 verticalSpeed = glm::cross(dir, glm::vec3(0.0f, 1.0f, 0.0f));
		if (verticalSpeed == glm::vec3(0.0f, 0.0f, 0.0f)) {
			verticalSpeed = glm::vec3(1.0f, 0.0f, 0.0f);
		}
		verticalSpeed = glm::normalize(verticalSpeed);
		float a = u(e) / 100.0f * 1.0f;
		verticalSpeed.x *= a;
		verticalSpeed.y *= a;
		verticalSpeed.z *= a;

		glm::vec3 verticalSpeed2 = glm::cross(dir, verticalSpeed);
		if (verticalSpeed2 == glm::vec3(0.0f, 0.0f, 0.0f)) {
			verticalSpeed2 = glm::vec3(0.0f, 0.0f, 1.0f);
		}
		verticalSpeed2 = glm::normalize(verticalSpeed2);
		float b = u(e) / 100.0f * 1.0f;
		verticalSpeed2.x *= b;
		verticalSpeed2.y *= b;
		verticalSpeed2.z *= b;

		// std::cout << "a: " << a << ", b: " << b << std::endl;

		// std::cout << "skill" << std::endl;
		blt->setPosProp(translate, skillScale);
		blt->setMotionProp(bulletSpeed * glm::normalize(dir) + verticalSpeed + verticalSpeed2, GRAVITY_ACC);
		blt->setBulletProp(group, skillDamage);
		blt->color = skillColor;

		skillInterval = 0.0f;
	}

	void processKeyboard(CameraMovement direction, float deltaTime) {
		glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
		float velocity = movementSpeed * deltaTime;
		if (direction == FORWARD)
			translate += front * velocity;
		if (direction == BACKWARD)
			translate -= front * velocity;
		if (direction == LEFT)
			translate -= right * velocity;
		if (direction == RIGHT)
			translate += right * velocity;
	}

	void update() {
		float currTime = glfwGetTime();
		deltaTime = currTime - lastTime;
		lastTime = currTime;
		skillInterval += deltaTime; 
		shootInterval += deltaTime; 
		shootRightInterval += deltaTime; 

		translate += speed * deltaTime;
		translate.y = yValue; 
		speed += acceleration * deltaTime;
	}

};