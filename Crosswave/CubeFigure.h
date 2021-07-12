#pragma once

#include "CubeRenderable.h"
#include "FigureObject.h"

class CubeFigure : public CubeRenderable, public FigureObject {

public: 
	CubeFigure(Group group, float hpMax) : FigureObject(group, hpMax), CubeRenderable() {
		
	}

	~CubeFigure() {

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

		glDrawArrays(GL_TRIANGLES, 0, 36);

		updateObject();

	}
};