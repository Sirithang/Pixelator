#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

const int DEFAULT_WIDTH = 1280;
const int DEFAULT_HEIGHT = 720;

GLFWwindow* CreateWindow();
GLuint LoadDefaultShaders();


struct RenderTarget
{
	GLuint framebuffer;
	GLuint texture;
	GLuint depthBuffer;

	int w, h;
};

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 uv;
};

namespace rendertarget
{
	void init(RenderTarget* rt, int w, int h);
	void bind(RenderTarget* rt);
}