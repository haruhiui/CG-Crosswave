#pragma once

#include <iostream> 
#include <vector> 
#include <list> 

#include <glad/glad.h>
#include <GLFW/glfw3.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "util.h" 

class SimpleBullet {

private: 
	// time 
	float lastTime, deltaTime;

public: 
	// basic property 
	glm::vec3 translate; 
	glm::vec3 scale; 

	// motion property 
	glm::vec3 speed; 
	glm::vec3 acceleration; 

	// color 
	glm::vec3 color; 

	// bullet property 
	bool hit;
	Group group; 
	float damage; 

public: 
	SimpleBullet(glm::vec3 translate = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f)) :
		translate(translate), scale(scale) {
		lastTime = glfwGetTime();
		deltaTime = 0; 

		speed = glm::vec3(0.0f); 
		acceleration = glm::vec3(0.0f); 

		color = glm::vec3(1.0f); 

		hit = false; 
		group = Group::FRIEND; 
		damage = 1.0f; 
	}

	void setPosProp(glm::vec3 t, glm::vec3 s) {
		translate = t;
		scale = s;
	}

	void setMotionProp(glm::vec3 v, glm::vec3 a) {
		speed = v; 
		acceleration = a; 
	}

	void setBulletProp(Group group, float damage) {
		this->group = group; 
		this->damage = damage; 
	}

	void update() {
		float currTime = glfwGetTime(); 
		deltaTime = currTime - lastTime; 
		lastTime = currTime; 

		translate += speed * deltaTime;
		speed += acceleration * deltaTime;
	}
};