#define GLEW_STATIC

#include "graphic.h"
#include "ui.h"
#include "imgui_impl_glfw_gl3.h"
#include <stdio.h>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

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

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint MatrixWorldID = glGetUniformLocation(programID, "M");

	//======= ASSIMP TEMP TEST CODE? MOVE TO OTHER FILE LATER =======

	//option import generate loads of stuff like tangent, normal etc.. so let's assume they are here
	const aiScene* scene = aiImportFile("bin/Debug/test.fbx", aiProcessPreset_TargetRealtime_MaxQuality);

	aiMesh* mesh = scene->mMeshes[0];
	int numVertex = mesh->mNumVertices;
	int numFaces = mesh->mNumFaces;

	Vertex* vertex = new Vertex[numVertex];
	unsigned int* indices = new unsigned int[numFaces * 3];

	for (int i = 0; i < numVertex; ++i)
	{
		vertex[i].position = *((glm::vec3*)(&mesh->mVertices[i]));
		vertex[i].normal = *((glm::vec3*)(&mesh->mNormals[i]));
		vertex[i].uv = *((glm::vec3*)(&mesh->mTextureCoords[0][i]));
	}

	for (int i = 0; i < numFaces; ++i)
	{
		indices[i * 3 + 0] = mesh->mFaces[i].mIndices[0];
		indices[i * 3 + 1] = mesh->mFaces[i].mIndices[1];
		indices[i * 3 + 2] = mesh->mFaces[i].mIndices[2];
	}
	
	GLuint vertexbuffer, indexBuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * numVertex, vertex, GL_STATIC_DRAW);

	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * numFaces * 3, indices, GL_STATIC_DRAW);
	//===============================================================

	/*static const GLfloat g_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		0.0f,  1.0f, 0.0f,
	};

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);*/


	RenderTarget pixelRT;
	rendertarget::init(&pixelRT, 128, 128);

	//----- App Data ----

	MainMenuState mainMenuState;
	SpriteViewerData sprViewerData;

	sprViewerData.cameraPosition = glm::vec3(0, -15, 12);
	sprViewerData.cameraTarget = glm::vec3(0, 0, 6);

	sprViewerData.associatedRT = &pixelRT;

	// Projection matrix : 45deg Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), pixelRT.w / (float)pixelRT.h, 0.1f, 100.0f);

	//===================

	bool spriteRenderOpen = true;
	while (!glfwWindowShouldClose(window))
	{
		float ratio;
		int width, height;

		ImGui_ImplGlfwGL3_NewFrame();

		DrawMainMenu(&mainMenuState);
		DrawSpriteViewer(&sprViewerData);
		
		// Camera matrix
		glm::mat4 View = glm::lookAt(
			sprViewerData.cameraPosition, // Camera is at (4,3,3), in World Space
			sprViewerData.cameraTarget, // and looks at the origin
			glm::vec3(0, 0, 1)  // Head is up (set to 0,-1,0 to look upside-down)
		);
		// Model matrix : an identity matrix (model will be at the origin)
		glm::mat4 Model = glm::mat4(1.0f);
		Model = glm::scale(Model, glm::vec3(0.1,0.1,0.1));
		// Our ModelViewProjection : multiplication of our 3 matrices
		glm::mat4 MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around

		rendertarget::bind(&pixelRT);
		
		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixWorldID, 1, GL_FALSE, &Model[0][0]);


		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			sizeof(Vertex),                  // stride
			(GLvoid*)0           // array buffer offset
		);

		glVertexAttribPointer(
			1,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			sizeof(Vertex),                  // stride
			(GLvoid*)sizeof(glm::vec3)           // array buffer offset
		);

		glVertexAttribPointer(
			2,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			sizeof(Vertex),                  // stride
			(GLvoid*)(2 * sizeof(glm::vec3))            // array buffer offset
		);

		// Draw the triangle !
		//glDrawArrays(GL_TRIANGLES, 0, scene->mMeshes[0]->mNumVertices); // 3 indices starting at 0 -> 1 triangle

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

		// Draw the triangles !
		glDrawElements(
			GL_TRIANGLES,      // mode
			numFaces * 3,    // count
			GL_UNSIGNED_INT,   // type
			(void*)0           // element array buffer offset
		);

		glDisableVertexAttribArray(0);
		
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