#pragma once
#include "VectorClasses.h"  

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
// #include <freeglut/freeglut.h> 


class Fluid
{
private:

	long            width; //宽
	long            height; //高

	Vector3D* buffer[2]; //缓冲区
	long            renderBuffer;  //当前渲染的缓冲区

	//Vector3D        *normal; //法线
	//Vector3D        *tangent; //切线

	int* indices[3]; //索引
	float* texcoords[3][2]; //纹理坐标
	float           k1, k2, k3; //多项式系数

	int texture;
public:

	//n:网格宽 m：网格高 d：网格点之间的距离
	//t:时间 c：波速 mu：阻力系数
	//mu 阻力系数
	Fluid(long n, long m, float d, float t, float c, float mu, int tex);
	~Fluid();

	void Evaluate(void);
	void draw();
};



Fluid::Fluid(long n, long m, float d, float t, float c, float mu, int tex)
{

	texture = tex;
	width = n;
	height = m;
	long count = n * m;
	buffer[0] = new Vector3D[count];
	buffer[1] = new Vector3D[count];
	renderBuffer = 0;


	for (int i = 0; i < 3; i++) {
		indices[i] = new int[2 * (n - 1) * (m - 1)];
	}
	for (int i = 0; i < 6; i++) {
		texcoords[i % 3][i / 3] = new float[2 * (n - 1) * (m - 1)];
	}


	float f1 = c * c * t * t / (d * d);
	float f2 = 1.0F / (mu * t + 2);
	k1 = (4.0F - 8.0F * f1) * f2;
	k2 = (mu * t - 2) * f2;
	k3 = 2.0F * f1 * f2;

	long a = 0;

	for (long j = 0; j < m; j++)
	{
		float y = d * j;
		for (long i = 0; i < n; i++)
		{
			if (i == 0 || j == 0 || i == n - 1 || j == m - 1)buffer[0][a].Set(d * i, y, 0.0F);
			else {
				int r = rand() % 2;
				if (r == 0)buffer[0][a].Set(d * i, y, 1.0F);
				else buffer[0][a].Set(d * i, y, 0.0F);
			}
			buffer[1][a] = buffer[0][a];
			a++;
		}
	}

	a = 0;
	float w = 1.0f / width;
	float h = 1.0f / height;
	for (int i = 0; i < n - 1; i++) {
		for (int j = 0; j < m - 1; j++) {
			long t = i + j * m;
			indices[0][a] = t;
			indices[1][a] = t + 1;
			indices[2][a] = t + n;

			texcoords[0][0][a] = i * w;
			texcoords[0][1][a] = j * h;

			texcoords[1][0][a] = (i + 1) * w;
			texcoords[1][1][a] = j * h;

			texcoords[2][0][a] = i * w;
			texcoords[2][1][a] = (j + 1) * h;

			a++;
		}
	}

	for (int i = 1; i < n; i++) {
		for (int j = 1; j < m; j++) {
			long t = i + j * m;
			indices[0][a] = t;
			indices[1][a] = t - 1;
			indices[2][a] = t - n;

			texcoords[0][0][a] = i * w;
			texcoords[0][1][a] = j * h;

			texcoords[1][0][a] = (i - 1) * w;
			texcoords[1][1][a] = j * h;

			texcoords[2][0][a] = i * w;
			texcoords[2][1][a] = (j - 1) * h;

			a++;
		}
	}
}

Fluid::~Fluid()
{
	delete[] buffer[1];
	delete[] buffer[0];
}

void Fluid::Evaluate(void)
{

	for (long j = 1; j < height - 1; j++)
	{
		const Vector3D* crnt = buffer[renderBuffer] + j * width;
		Vector3D* prev = buffer[1 - renderBuffer] + j * width;

		for (long i = 1; i < width - 1; i++)
		{
			prev[i].z = k1 * crnt[i].z + k2 * prev[i].z +
				k3 * (crnt[i + 1].z + crnt[i - 1].z +
					crnt[i + width].z + crnt[i - width].z);
		}
	}
	renderBuffer = 1 - renderBuffer;
}

void Fluid::draw()
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
	glBegin(GL_TRIANGLES);

	for (int i = 0; i < 2 * (height - 1) * (width - 1); i++) {
		glTexCoord2f(texcoords[0][0][i], texcoords[0][1][i]);

		glVertex3f(buffer[renderBuffer][indices[0][i]].x,
			buffer[renderBuffer][indices[0][i]].y,
			buffer[renderBuffer][indices[0][i]].z);

		glTexCoord2f(texcoords[1][0][i], texcoords[1][1][i]);
		glVertex3f(buffer[renderBuffer][indices[1][i]].x,
			buffer[renderBuffer][indices[1][i]].y,
			buffer[renderBuffer][indices[1][i]].z);
			
		glTexCoord2f(texcoords[2][0][i], texcoords[2][1][i]);
		glVertex3f(buffer[renderBuffer][indices[2][i]].x,
			buffer[renderBuffer][indices[2][i]].y,
			buffer[renderBuffer][indices[2][i]].z);
	}

	glEnd();
	glDisable(GL_TEXTURE_2D);
}