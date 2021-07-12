#pragma once

#include "Object.h"
#include "Renderable.h" 

class CubeRenderable : public Renderable {

public: 
	CubeRenderable() : Renderable() {
        vertices = {
            // positions          
            -0.5f, -0.5f, -0.5f,  
             0.5f, -0.5f, -0.5f,  
             0.5f,  0.5f, -0.5f,  
             0.5f,  0.5f, -0.5f,  
            -0.5f,  0.5f, -0.5f,  
            -0.5f, -0.5f, -0.5f,  

            -0.5f, -0.5f,  0.5f,  
             0.5f, -0.5f,  0.5f,  
             0.5f,  0.5f,  0.5f,  
             0.5f,  0.5f,  0.5f,  
            -0.5f,  0.5f,  0.5f,  
            -0.5f, -0.5f,  0.5f,  

            -0.5f,  0.5f,  0.5f,  
            -0.5f,  0.5f, -0.5f,  
            -0.5f, -0.5f, -0.5f,  
            -0.5f, -0.5f, -0.5f,  
            -0.5f, -0.5f,  0.5f,  
            -0.5f,  0.5f,  0.5f,  

             0.5f,  0.5f,  0.5f,  
             0.5f,  0.5f, -0.5f,  
             0.5f, -0.5f, -0.5f,  
             0.5f, -0.5f, -0.5f,  
             0.5f, -0.5f,  0.5f,  
             0.5f,  0.5f,  0.5f,  

            -0.5f, -0.5f, -0.5f,  
             0.5f, -0.5f, -0.5f,  
             0.5f, -0.5f,  0.5f,  
             0.5f, -0.5f,  0.5f,  
            -0.5f, -0.5f,  0.5f,  
            -0.5f, -0.5f, -0.5f,  

            -0.5f,  0.5f, -0.5f,  
             0.5f,  0.5f, -0.5f,  
             0.5f,  0.5f,  0.5f,  
             0.5f,  0.5f,  0.5f,  
            -0.5f,  0.5f,  0.5f,  
            -0.5f,  0.5f, -0.5f
        };

        for (int i = 0; i < vertices.size(); i++) {
            vertices[i] *= 2.0f; 
        }
	}

    ~CubeRenderable() {

    }
};