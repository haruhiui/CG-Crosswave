#pragma once

#include <iostream> 
#include <vector> 
#include <list> 

#include <glad/glad.h>
#include <GLFW/glfw3.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "util.h" 
#include "SimpleBullet.h"

class SimpleFigure {

private: 
	// time 
	float lastTime, deltaTime; 

public: 
	// basic property 
	glm::vec3 translate, scale; 

	// motion property 
	glm::vec3 speed, acceleration; 

	// game property 
	Group group;
	float hp, maxHp; 

public:
	SimpleFigure(glm::vec3 translate = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f)) :
		translate(translate), scale(scale) {
		lastTime = glfwGetTime();
		deltaTime = 0;

		speed = glm::vec3(0.0f);
		acceleration = glm::vec3(0.0f);

		hp = maxHp = 10.0f; 
		group = Group::FRIEND;
	}

	void setPosProp(glm::vec3 t, glm::vec3 s) {
		translate = t; 
		scale = s;
	}

	void setMotionProp(glm::vec3 v, glm::vec3 a) {
		speed = v;
		acceleration = a;
	}

	void setFigureProp(Group group, float maxHp) {
		this->group = group;
		this->maxHp = maxHp; 
		this->hp = maxHp; 
	}

	// 自动判断子弹是否命中，命中则减去blt->damage的hp并设置blt->hit为true
	void hitOrMiss(SimpleBullet* blt) {
		glm::vec3 r = blt->translate - this->translate; 
		float dist = glm::dot(r, r); 
		r = blt->scale + this->scale; 
		float hitDist = glm::dot(r, r); 
		if (dist < hitDist) {
			hp -= blt->damage; 
			blt->hit = true; 
		}
	}

	void update() {
		float currTime = glfwGetTime();
		deltaTime = currTime - lastTime;
		lastTime = currTime;

		translate += speed * deltaTime;
		speed += acceleration * deltaTime;
	}
};
