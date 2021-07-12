#pragma once

#include <iostream> 
#include <vector> 
#include <list> 

#include <glad/glad.h>
#include <ctime>
#include <GLFW/glfw3.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "Camera.h" 
#include "Window.h"
#include "Object.h" 

#include "SimpleBullet.h"
#include "GeneralBullet.h"

/*
 * 继承自Object，表示人物
 * 有所属阵营、hpMax、hp
 * hp归零后的注销由产生的地方进行
 */
class FigureObject : public Object {

protected:
	Group group;

	float hpMax, hp;

	bool skillReady = false; 
	float skillInterval = 0.0f; 

public:
	FigureObject(Group group, float hpMax) : Object() {
		this->group = group;
		this->hpMax = hpMax;
		this->hp = hpMax;
	}

	~FigureObject() {

	}

	bool isAlive() {
		return hp > 0;
	}

	// 传入一个子弹blt，如果命中则将自己的hp减掉blt的damage，并设置blt的hit
	void bulletCrash(BulletObject& blt) {
		if (Object::isCrash(blt)) {
			hp -= blt.getDamage();
			blt.hitObj();
			std::cout << "hit figure, remain hp: " << hp << std::endl; 
		}
	}

	// 获得子弹的初速度
	float getBulletSpeed() {
		return 20.0f; 
	}

	// 射击，将传进来的子弹blt初始化，固定一个速度(glm::vec3)即可
	void shoot(SimpleBullet* blt, glm::vec3 dir) {
		std::cout << "shoot" << std::endl;
		blt->translate = this->pos.translateVec;
		blt->scale = glm::vec3(0.1f); 
		blt->color = glm::vec3(15.0f, 7.0f, 0.0f); 

		blt->setBulletProp(Group::FRIEND, 1.0f);
		blt->setMotionProp(dir * getBulletSpeed(), GRAVITY_ACC); 
	}

	int getShootRightNum() {
		return 3; 
	}

	// 右键射击
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

		std::cout << "shoot right" << std::endl; 
		blt->translate = this->pos.translateVec + verticalPos; 
		blt->translate.y = 0;	// 设置在海面上
		blt->scale = glm::vec3(0.1f);
		blt->color = glm::vec3(15.0f, 7.0f, 0.0f);

		blt->setBulletProp(Group::FRIEND, 1.0f); 
		blt->setMotionProp(speed * getBulletSpeed(), glm::vec3(0.0f)); 
	}

	bool isSkillReady() {
		static float lastTime, deltaTime; 
		float currTime = glfwGetTime(); 
		deltaTime = currTime - lastTime; 
		lastTime = currTime; 
		skillInterval += deltaTime; 

		bool printed = false; 

		if (skillInterval > 10.0f && !skillReady) {
			skillReady = true; 
		}
		return skillReady; 
	}

	int getSkillNum() {
		return 10; 
	}

	// 技能
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

		std::cout << "a: " << a << ", b: " << b << std::endl; 

		std::cout << "skill" << std::endl; 
		blt->translate = this->pos.translateVec; 
		blt->scale = glm::vec3(0.1f); 
		blt->color = glm::vec3(15.0f, 0.0f, 0.0f); 
		
		blt->setBulletProp(Group::FRIEND, 1.0f); 
		blt->setMotionProp(dir * getBulletSpeed() + verticalSpeed + verticalSpeed2, GRAVITY_ACC); 

		skillReady = false; 
		skillInterval = 0.0f; 
	}

};

