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
 * �̳���Object����ʾ����
 * ��������Ӫ��hpMax��hp
 * hp������ע���ɲ����ĵط�����
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

	// ����һ���ӵ�blt������������Լ���hp����blt��damage��������blt��hit
	void bulletCrash(BulletObject& blt) {
		if (Object::isCrash(blt)) {
			hp -= blt.getDamage();
			blt.hitObj();
			std::cout << "hit figure, remain hp: " << hp << std::endl; 
		}
	}

	// ����ӵ��ĳ��ٶ�
	float getBulletSpeed() {
		return 20.0f; 
	}

	// ����������������ӵ�blt��ʼ�����̶�һ���ٶ�(glm::vec3)����
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

	// �Ҽ����
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

		// ��speedӳ�䵽xzƽ�棬���ִ�С����
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
		blt->translate.y = 0;	// �����ں�����
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

	// ����
	void skill(SimpleBullet* blt, glm::vec3 dir) {
		// ����speed������ʮ���ӵ�����������������
		static std::default_random_engine e(time(NULL));
		// e.seed(time(NULL));				// ����������ӣ�����ÿ��ѭ�������Ķ���һ����
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

