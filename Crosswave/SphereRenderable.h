#pragma once

#include "Object.h"
#include "Renderable.h"

// 表示一个球形物体
class SphereRenderable : public Renderable {

protected:
	int xSeg, ySeg;

public:
	SphereRenderable(int xSeg = 50, int ySeg = 50) : Renderable() {
		this->xSeg = xSeg;
		this->ySeg = ySeg;

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

		enableIndices();
	}

	~SphereRenderable() {

	}

};