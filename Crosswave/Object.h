/* 
 * last modify: 2020.12.29 
 * ADD 		PositionProperty, MotionProperty, Object::update 
 * CHANGE 	Object() 
 */ 

#pragma once

#include <iostream> 
#include <vector> 

#include <glad/glad.h>
#include <GLFW/glfw3.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "util.h" 
#include "Shader.h"
#include "Camera.h" 

struct PositionProperty {

	glm::vec3 translateVec; 
	glm::vec3 rotateVec; 
	glm::vec3 scaleVec; 
	float rotateDeg; 

	PositionProperty(
		glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3 rotate = glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f),
		float rotDeg = 0.0f) :
		translateVec(position),
		rotateVec(rotate),
		scaleVec(scale),
		rotateDeg(rotDeg) {

		assert(rotate != glm::vec3(0.0f, 0.0f, 0.0f));
		assert(scale != glm::vec3(0.0f, 0.0f, 0.0f));
	}

}; 


struct MotionProperty {

    glm::vec3 acceleration;     // ���ٶ�
    glm::vec3 speed;            // һ���ٶ�
    float quality;          	// ��������

	MotionProperty(
		float m = 1.0f, 
		glm::vec3 v = glm::vec3(0.0f, 0.0f, 0.0f), 
		glm::vec3 acc = glm::vec3(0.0f, 0.0f, 0.0f)) : 
		quality(m), 
		speed(v), 
		acceleration(acc) {

		assert(quality != 0.0f); 
	}

}; 

// �����������ʱ����
struct VolumeProperty {

};

/*
 * ��Ӽ򵥵�������ײ��ⷽ��isCrash(const Object& obj); 
 */
class Object {

private: 
	float lastTime, deltaTime = 0; 

protected:
	bool visible;					// �����ɼ�

	PositionProperty pos; 
	MotionProperty motion; 

	float radius; 

public:
	Object() {
		visible = true;
		lastTime = glfwGetTime(); 
		radius = 1.0f; 
	}

	~Object() {

	}

	// gets 

	PositionProperty& getPos() {
		return pos; 
	}

	MotionProperty& getMotion() {
		return motion;
	}

	float getRadius() {
		return radius;
	}

	bool getVisible() {
		return visible;
	}

	// sets 

	void setPos(const PositionProperty& pos) {
		this->pos = pos; 
	}

	void setMotion(const MotionProperty& prop) {
		this->motion = prop; 
	}

	void setCrashRadius(float r) {
		radius = r; 
	}

	void setVisible(bool vis) {
		this->visible = vis;
	}

	// update 
	void updateObject() {
		// update time 
		float currTime = glfwGetTime(); 
		deltaTime = currTime - lastTime; 
		lastTime = currTime; 
		
		// update pos by motion 
		pos.translateVec += motion.speed * deltaTime; 
		
		// update motion 
		motion.speed += motion.acceleration * deltaTime; 
	}

	// crash 
	bool isCrash(const Object& obj) {
		glm::vec3 distVec = pos.translateVec - obj.pos.translateVec; 
		
		//std::cout << "my:  " << pos.translateVec.x << " " << pos.translateVec.y << " " << pos.translateVec.z << std::endl; 
		//std::cout << "obj: " << obj.pos.translateVec.x << " " << obj.pos.translateVec.y << " " << obj.pos.translateVec.z << std::endl; 
		//std::cout << distVec.x << " " << distVec.y << " " << distVec.z << std::endl; 
		float distValue = distVec.x * distVec.x + distVec.y * distVec.y + distVec.z * distVec.z; 
		float radiusSum = radius + obj.radius; 
		float radiusPow = radiusSum * radiusSum;

		//std::cout << "rsdius: " << radius << ", obj.radius: " << obj.radius << std::endl; 
		//std::cout << "diatValue: " << distValue << ", radiusPow: " << radiusPow << std::endl; 
		if (distValue <= radiusPow) {
			return true; 
		}
		else {
			return false; 
		}
	}
};

