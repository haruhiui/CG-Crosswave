#pragma once

#include <Windows.h> 
#define BITMAP_ID 0x4D42  

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

// constants 
const glm::vec3 GRAVITY_ACC = glm::vec3(0.0f, -3.0f, 0.0f);		// 重力加速度
const float PI = 3.14159265358979323846f;

unsigned int loadTexture(char const* path, bool gammaCorrection);
void renderQuad();
void renderCube();
void renderSphere(); 

enum class Group {
    FRIEND,
    ENEMY
};

// 判断这两个方向是否基本重合，用于瞄准检测
bool isAimed(glm::vec3 r1, glm::vec3 r2, float threshold) {
    // std::cout << glm::dot(glm::normalize(r1), glm::normalize(r2)) << std::endl; 
    return glm::dot(glm::normalize(r1), glm::normalize(r2)) > threshold; 
}

// 通过目标和源坐标来根据抛物线确定抛出的速度
glm::vec3 calcParabolaSpeed(glm::vec3 src, glm::vec3 dist, float v, float g) {
    glm::vec3 toDist = dist - src; 
    float x = sqrt(glm::dot(toDist, toDist)); 
    float tanTheta = (v * v + sqrt(v * v * v * v - g * g * x)) / (g * x);   // 根据抛物线轨迹计算角度
    float angle = atan(tanTheta); 

    glm::vec3 right = glm::cross(glm::normalize(toDist), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotMat = glm::mat4(1.0f); 
    rotMat = glm::rotate(rotMat, angle, right); 
    glm::vec3 res = rotMat * glm::vec4(glm::normalize(toDist), 1.0f); 
    return glm::normalize(res) * v; 
}


glm::vec3 getShootUp(glm::vec3 src, glm::vec3 dist, float speed, float gravity, float height) {
    glm::vec3 toDist = dist - src;
    float x = sqrt(glm::dot(toDist, toDist));
    float time = x / speed; 
    return glm::vec3(0.0f, 1.0f, 0.0f) * gravity * (time / 2); 
}


// utility function for loading a 2D texture from file
unsigned int loadTexture(char const* path, bool gammaCorrection) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum internalFormat;
        GLenum dataFormat;
        if (nrComponents == 1)
        {
            internalFormat = dataFormat = GL_RED;
        }
        else if (nrComponents == 3)
        {
            internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
            dataFormat = GL_RGB;
        }
        else if (nrComponents == 4)
        {
            internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
            dataFormat = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}


// renderCube() renders a 1x1 3D cube in NDC.
void renderCube() {
    static unsigned int cubeVAO = 0;
    static unsigned int cubeVBO = 0;

    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}


// renderQuad() renders a 1x1 XY quad in NDC
void renderQuad() {
    static unsigned int quadVAO = 0;
    static unsigned int quadVBO;

    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void renderSphere() { 
    static unsigned int sphereVAO = 0;
    static unsigned int sphereVBO;
    static unsigned int sphereEBO; 

    const int xSeg = 50, ySeg = 50;
    static Shader testShader("general_bullet.vs", "general_bullet.fs");
    
    if (sphereVAO == 0) {
        std::vector<float> vertices;
        std::vector<int> indices;

        // vertices 
        for (int y = 0; y <= ySeg; y++) {
            for (int x = 0; x <= xSeg; x++) {
                float xSegment = (float)x / (float)xSeg;
                float ySegment = (float)y / (float)ySeg;
                float xPosition = std::sin(ySegment * PI) * std::cos(xSegment * 2.0f * PI);
                float yPosition = std::cos(ySegment * PI);
                float zPosition = std::sin(ySegment * PI) * std::sin(xSegment * 2.0f * PI);

                vertices.push_back(xPosition);
                vertices.push_back(yPosition);
                vertices.push_back(zPosition);
            }
        }
        // indices 
        for (int i = 0; i < ySeg; i++) {
            for (int j = 0; j < xSeg; j++) {
                indices.push_back(i * (xSeg + 1) + j);
                indices.push_back((i + 1) * (xSeg + 1) + j);
                indices.push_back((i + 1) * (xSeg + 1) + j + 1);

                indices.push_back(i * (xSeg + 1) + j);
                indices.push_back((i + 1) * (xSeg + 1) + j + 1);
                indices.push_back(i * (xSeg + 1) + j + 1);
            }
        }
        glGenVertexArrays(1, &sphereVAO);
        glGenBuffers(1, &sphereVBO);
        glGenBuffers(1, &sphereEBO);
        glBindVertexArray(sphereVAO);
        // fill VBO
        glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
        // fill EBO 
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_STATIC_DRAW);
        // link vertex attributes 
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);		// default 
        // 
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0); 
    }
    // render sphere 
    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLES, xSeg * ySeg * 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void renderCycle() {
    static unsigned int cycleVAO = 0; 
    static unsigned int cycleVBO = 0; 
    
    const int r = 5.0f; 
    if (cycleVAO == 0) {
        float vertices[300]; 
        for (int i = 0; i < 100; i++) {
            vertices[i * 3] = r * cos(i * PI / 50); 
            vertices[i * 3 + 1] = r * sin(i * PI / 50); 
            vertices[i * 3 + 2] = 0.0f; 
            // setup plane VAO
            glGenVertexArrays(1, &cycleVAO);
            glGenBuffers(1, &cycleVBO);
            glBindVertexArray(cycleVAO);
            glBindBuffer(GL_ARRAY_BUFFER, cycleVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glBindVertexArray(0); 
        }
    }

    glBindVertexArray(cycleVAO); 
    glDrawArrays(GL_LINE_LOOP, 0, 100); 
    // glDrawArrays(GL_TRIANGLE_STRIP, 0, 100);
    glBindVertexArray(0); 
}




// 下面是CGwatersurface中的texture.cpp内容
//
//static unsigned char* LoadBitmapFile(char* filename, BITMAPINFOHEADER* bitmapInfoHeader)
//{
//
//    FILE* filePtr;
//    BITMAPFILEHEADER bitmapFileHeader;
//    unsigned char* bitmapImage;
//    int imageIdx = 0;
//    unsigned char tempRGB;
//    filePtr = fopen(filename, "rb");
//    if (filePtr == NULL) {
//        printf("file not open\n");
//        return NULL;
//    }
//
//    fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
//    if (bitmapFileHeader.bfType != BITMAP_ID) {
//        fprintf(stderr, "Error in LoadBitmapFile: the file is not a bitmap file\n");
//        return NULL;
//    }
//    fread(bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
//    fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);
//    bitmapImage = new unsigned char[bitmapInfoHeader->biSizeImage];
//    if (!bitmapImage) {
//        fprintf(stderr, "Error in LoadBitmapFile: memory error\n");
//        return NULL;
//    }
//
//    fread(bitmapImage, 1, bitmapInfoHeader->biSizeImage, filePtr);
//    if (bitmapImage == NULL) {
//        fprintf(stderr, "Error in LoadBitmapFile: memory error\n");
//        return NULL;
//    }
//    for (imageIdx = 0; imageIdx < bitmapInfoHeader->biSizeImage; imageIdx += 3) {
//        tempRGB = bitmapImage[imageIdx];
//        bitmapImage[imageIdx] = bitmapImage[imageIdx + 2];
//        bitmapImage[imageIdx + 2] = tempRGB;
//    }
//    fclose(filePtr);
//    return bitmapImage;
//}
//
//void loadTex(int i, char* filename, GLuint* texture)
//{
//
//    BITMAPINFOHEADER bitmapInfoHeader;
//    unsigned char* bitmapData;
//
//    bitmapData = LoadBitmapFile(filename, &bitmapInfoHeader);
//    glEnable(GL_TEXTURE_2D);
//
//    glBindTexture(GL_TEXTURE_2D, texture[i]);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//    std::cout << bitmapInfoHeader.biWidth << " " << bitmapInfoHeader.biHeight << std::endl;
//    std::cout << sizeof(bitmapData) << std::endl;
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bitmapInfoHeader.biWidth, bitmapInfoHeader.biHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, bitmapData);
//
//}
