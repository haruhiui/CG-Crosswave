#pragma once

#include <iostream>
#include <vector> 

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif 

#include "Shader.h" 
#include "Camera.h" 
#include "Window.h"

/*
 * 基本的可渲染物体
 * 在循环外：setShader, bindVertexArray，如果有纹理还需要：addTextureImage, bindTexture
 * 在循环内：render 
 * 继承自Renderable的类需要重写renderObject方法用来渲染自己
 */
class Renderable {

protected: 

	unsigned int vao, vbo, ebo, *tex = NULL; 

	Shader* shader;
	
	bool indicesEnabled = false; 
	bool textureEnabled = false; 

	std::vector<float> vertices; 
	std::vector<int> indices; 
	std::vector<std::string> textureImages; 

public: 
	Renderable() {
		vao = vbo = ebo = 0; 
		shader = NULL; 
	}

	~Renderable() {

	}

	// 
	bool isIndicesEnabled() {
		return indicesEnabled; 
	}

	void enableIndices() {
		indicesEnabled = true; 
	}

	void disableIndices() {
		indicesEnabled = false; 
	}

	// 
	bool isTextureEnabled() {
		return textureEnabled;
	}

	void enableTexture() {
		textureEnabled = true; 
	}

	void disableTexture() {
		textureEnabled = false; 
	}

	// 
	const std::vector<float>& getVertices() {
		return vertices;
	}

	const std::vector<int>& getIndices() {
		return indices;
	}

	// 
	void setShader(Shader *shader) {
		this->shader = shader; 
	}

	virtual void bindVertexArray() {
		glGenVertexArrays(1, &vao); 
		glGenBuffers(1, &vbo); 

		glBindVertexArray(vao); 
		glBindBuffer(GL_ARRAY_BUFFER, vbo); 
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW); 

		if (isIndicesEnabled()) {
			glGenBuffers(1, &ebo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo); 
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_STATIC_DRAW); 
		}

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);		// default 

		glBindBuffer(GL_ARRAY_BUFFER, 0); 
		glBindVertexArray(0); 
	}

	void addTextureImage(std::string filename) {
		textureImages.push_back(filename);
	}

	virtual void bindTexture() {
		// 在调用bind之前要先启用textureEnabled
		if (!isTextureEnabled())	return; 

		tex = new unsigned int[textureImages.size()]; 
		glGenTextures(textureImages.size(), tex);

		assert(shader != NULL);
		shader->use(); 

		for (int i = 0; i < textureImages.size(); i++) {
			int width, height, nrChannels;
			stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
			
			unsigned char *data = stbi_load(textureImages[i].c_str(), &width, &height, &nrChannels, 0);
			if (data) {
				GLenum format;
				if (nrChannels == 1)		format = GL_RED;
				else if (nrChannels == 3)	format = GL_RGB;
				else if (nrChannels == 4)	format = GL_RGBA; 

				glBindTexture(GL_TEXTURE_2D, tex[i]);
				glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
				glGenerateMipmap(GL_TEXTURE_2D);

				// set the texture wrapping parameters 
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				// set texture filtering parameters 
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				stbi_image_free(data);

				shader->setInt("texture" + (i + 1), i);	// set uniforms 
				std::cout << "texture" + (i + 1) << " " << i << std::endl;
			}
			else {
				std::cout << "Failed to load texture" << std::endl;
				stbi_image_free(data);
			}
		}
	}

	virtual void render(const Window& window, const Camera& camera) {
		shader->use();

		for (int i = 0; i < textureImages.size(); i++) {
			glActiveTexture(GL_TEXTURE0 + i);			// This does work. 
			glBindTexture(GL_TEXTURE_2D, tex[i]); 
		}

		glBindVertexArray(vao);
		renderObject(window, camera);
	}

	virtual void renderObject(const Window& window, const Camera& camera) = 0;

};
