#pragma once

#include <iostream> 
#include <vector> 
#include <list> 

#include <glad/glad.h>
#include <GLFW/glfw3.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "util.h"
#include "Shader.h"
#include "Camera.h" 
#include "Window.h"
#include "Object.h" 


/*
 * 继承自Object，表示子弹
 * 有所属阵营、伤害值、是否击中
 * 命中后的注销由产生的地方进行
 */
class BulletObject : public Object {

protected:
	Group group;

	bool hit;
	float damage;

public:
	BulletObject(Group group, float damage) : Object() {
		this->group = group;
		this->damage = damage;
		this->hit = false;
	}

	~BulletObject() {

	}

	Group getGroup() {
		return group;
	}

	float getDamage() {
		return damage;
	}

	bool isHit() {
		return hit;
	}

	void hitObj() {
		hit = true;
	}
};

