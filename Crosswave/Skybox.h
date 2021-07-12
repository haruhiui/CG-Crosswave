#pragma once

#include <iostream> 
#include <vector> 

#include <glad/glad.h>
#include <GLFW/glfw3.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "Window.h"
#include "Camera.h"
#include "Renderable.h"

/*
 * 直接继承自Renderable，假装实现了renderObject，可直接实例化
 */

class Skybox : public Renderable {

protected:
	std::vector<std::string> faces; 

public: 
	Skybox(std::vector<std::string>& faces) : Renderable() {
		this->faces = faces; 

		// vertices           
		vertices = {
			-1.0f,  1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			-1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f
		};
	}

	~Skybox() {

	}

	void bindTexture() {
		tex = new unsigned int[1]; 
		glGenTextures(1, tex); 
		glBindTexture(GL_TEXTURE_CUBE_MAP, tex[0]); 

		int width, height, nrChannels; 
		for (unsigned int i = 0; i < faces.size(); i++) {
			unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0); 
			if (data) {
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data); 
				stbi_image_free(data); 
			}
			else {
				std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl; 
				stbi_image_free(data); 
			}
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		shader->use(); 
		shader->setInt("skybox", 0); 
	}

	void render(const Window& window, const Camera& camera) {
		glDepthFunc(GL_LEQUAL);	// change depth function so depth test passes when values are equals to depth buffer's content 

		shader->use();
		glm::mat4 view = glm::mat4(glm::mat3(camera.getViewMatrix()));	// remove translation from the view matrix 
		glm::mat4 projection = glm::perspective(glm::radians(camera.getZoom()), (float)window.scrWidth / (float)window.scrHeight, 0.1f, 100.0f);
		shader->setMat4("view", view);
		shader->setMat4("projection", projection); 

		glActiveTexture(GL_TEXTURE0); 
		glBindTexture(GL_TEXTURE_CUBE_MAP, tex[0]);

		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36); 
		
		glBindVertexArray(0); 
		glDepthFunc(GL_LESS); 
	}

	// overwrite renderObject with nothing so that we can instantiate a Skybox 
	void renderObject(const Window& window, const Camera& camera) {

	}

};