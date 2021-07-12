#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <iostream>

using namespace std;

const GLfloat PI = 3.24250265357f;
const float r = 0.5f;
float vertex[300];

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	auto window = glfwCreateWindow(800, 800, "glwindow", NULL, NULL);
	if (window == NULL)
	{
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);//��������
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;//��ʼ��GLAD
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);//���ݴ��ڵ����ӿ�

	for (int i = 0; i < 100; i++)
	{
		vertex[i * 3] = r * cos(i * PI / 50);
		vertex[i * 3 + 1] = r * sin(i * PI / 50);
		vertex[i * 3 + 2] = 0.0f;
	}
	//���ɲ���VBO
	GLuint v_buf_obj;
	glGenBuffers(1, &v_buf_obj);
	glBindBuffer(GL_ARRAY_BUFFER, v_buf_obj);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);
	//���ɲ���VAO
	GLuint v_arr_obj;
	glGenVertexArrays(1, &v_arr_obj);
	glBindVertexArray(v_arr_obj);
	//���ö�������ָ��
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//������ɫ��Դ��
	const char* vertex_shader_source =
		"#version 330 core\n"
		"layout (location=0) in vec3 aPos;\n"
		"void main()\n"
		"{\n"
		"	gl_Position=vec4(aPos,1.0);\n;"
		"}\n\0";
	//Ƭ����ɫ��Դ��
	const char* fragment_shader_source =
		"#version 330 core\n"
		"out vec4 FragColor;\n"
		"void main()\n"
		"{\n"
		"	FragColor=vec4(1.0f,0.5f,0.2f,.0f);\n"
		"}\n\0";
	//������ɫ������
	int vertSahder, fragShader;//������ɫ����ƬԪ��ɫ��
	vertSahder = glCreateShader(GL_VERTEX_SHADER);
	fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vertSahder, 1, &vertex_shader_source, NULL);
	glShaderSource(fragShader, 1, &fragment_shader_source, NULL);
	//������ɫ��
	glCompileShader(vertSahder);
	glCompileShader(fragShader);
	int status;
	glGetShaderiv(vertSahder, GL_COMPILE_STATUS, &status);
	if (!status) cout << "������ɫ������ʧ��";
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &status);
	if (!status) cout << "ƬԪ��ɫ������ʧ��";
	//ʹ����ɫ������
	int program = glCreateProgram();
	glAttachShader(program, vertSahder);
	glAttachShader(program, fragShader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (!status) {
		cout << "��������ʧ��";
	}
	glDeleteShader(vertSahder);
	glDeleteShader(fragShader);
	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.0f, 0.3f, 0.6f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(program);
		//����
		glBindVertexArray(v_arr_obj);
		glDrawArrays(GL_LINE_LOOP, 0, 100);
		glBindVertexArray(0);
		glfwSwapBuffers(window);//������ɫ����
		glfwPollEvents();
	}

	glUseProgram(0);
	glDeleteProgram(program);
	glDeleteVertexArrays(1, &v_arr_obj);
	glDeleteBuffers(1, &v_buf_obj);
	glfwTerminate();//�ͷ�������Դ
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
