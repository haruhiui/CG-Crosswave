#pragma once

#include <iostream> 
#include <vector> 
#include <list> 

#include <glad/glad.h>
#include <GLFW/glfw3.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "Camera.h" 
#include "Window.h"
#include "Object.h" 
#include "BulletObject.h"
#include "Renderable.h"
#include "SphereRenderable.h"

class GeneralBullet : public BulletObject, public SphereRenderable {

public: 
	GeneralBullet(Group group, float damage) : 
		BulletObject(group, damage), 
		SphereRenderable() {

	}

	~GeneralBullet() {

	}

	void renderObject(const Window& window, const Camera& camera) {

		glm::mat4 projection = glm::perspective(glm::radians(camera.getZoom()), (float)window.scrWidth / (float)window.scrHeight, 0.1f, 100.0f);
		shader->setMat4("projection", projection);

		glm::mat4 view = camera.getViewMatrix();
		shader->setMat4("view", view);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, pos.translateVec);
		model = glm::rotate(model, glm::radians(pos.rotateDeg), pos.rotateVec);
		model = glm::scale(model, pos.scaleVec);
		shader->setMat4("model", model);

		shader->setVec3("lightDir", glm::vec3(0.0f, 1.0f, 0.0f)); 
		shader->setVec3("viewPos", camera.getPosition()); 
		shader->setVec3("lightColor", glm::vec3(1.0f)); 
		shader->setVec3("objectColor", glm::vec3(0.7f, 0.3f, 0.3f)); 

		glDrawElements(GL_TRIANGLES, xSeg * ySeg * 6, GL_UNSIGNED_INT, 0);

		updateObject();
	}
};