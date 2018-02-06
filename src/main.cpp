#define GLEW_STATIC

#include "graphic.h"
#include "ui.h"
#include "imgui_impl_glfw_gl3.h"
#include <stdio.h>
#include <glm/gtc/matrix_transform.hpp>

int main()
{
	GLFWwindow* window = CreateWindow();

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Setup ImGui binding
	ImGui_ImplGlfwGL3_Init(window, true);

	// Setup style
	ImGui::StyleColorsClassic();

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadDefaultShaders();

	RenderTarget pixelRT;
	rendertarget::init(&pixelRT, 128, 128);

	//----- App Data ----

	MainMenuState mainMenuState;
	SpriteViewerData sprViewerData;
	Camera cam;
	Model model;

	cam.position = glm::vec3(0, 100, 250);
	cam.target = glm::vec3(0, 100, 0);

	model::fromFile(&model, "testData/test.fbx");

	sprViewerData.associatedRT = &pixelRT;
	sprViewerData.camera = &cam;

	// Projection matrix : 45deg Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), pixelRT.w / (float)pixelRT.h, 0.1f, 10000.0f);

	//===================
	double currentFrame, lastFrame, deltaTime;
	lastFrame = glfwGetTime();

	bool spriteRenderOpen = true;
	while (!glfwWindowShouldClose(window))
	{
		float ratio;
		int width, height;

		currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		ImGui_ImplGlfwGL3_NewFrame();

		DrawMainMenu(&mainMenuState);
		DrawSpriteViewer(&sprViewerData);
		
		// Camera matrix
		glm::mat4 View = glm::lookAt(
			cam.position, // Camera is at (4,3,3), in World Space
			cam.target, // and looks at the origin
			glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);

		cam.viewProjMat = Projection * View;

		rendertarget::bind(&pixelRT);
		
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glClearColor(1, 1, 1, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		model::tickAnimation(&model, deltaTime);
		model::render(&model, &cam);
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);
		
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui::Render();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();


	return 0;
}