#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <map>
#include <assimp/scene.h>

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

struct Camera
{
	glm::vec3 position;
	glm::vec3 target;

	glm::mat4 viewProjMat;
};

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 uv;

	glm::ivec4 bonesID;
	glm::vec4 weights;
};

struct Mesh
{
	GLuint vertexBuffer;
	GLuint indexBuffer;

	unsigned int indexCount;
};

struct Material
{
	GLuint program;

	//TODO : remove that to maybe make it a bit more flexible (even if this is not an engine)
	GLuint MVPLocation;
	GLuint MLocation;
};

//struct BoneData
//{
//	int parent;
//	int children[12];
//	int childCount;
//};

struct Model
{
	int meshCount;

	Mesh* meshArray;
	Material* materialArray;

	//map the node in the loaded model to their index into the node array.
	//useful to get which index in the bone array a specific vertex is bound to
	std::map<unsigned long, int> boneMapping;
	
	//hard limit, we only support 100 bones max in GPU anyway.
	//Transform is a separate array to upload it right away to GPU easily
	//TODO make that slightly more dynamic, right now, that take ~11 kB per model whatever the amount of bones..
	//BoneData bonesData[100];
	glm::mat4 boneTransform[100];

	//look like this can be different per mesh for multiple meshes scene, so should probably be stored on the mesh, TODO : investigate
	glm::mat4 boneOffsets[100];
	int boneCount;

	//Unhappy of storing the full scene here, but easier fo rnow. rewrite later to store only stripped down data we need (node hierarchy + animations)
	const aiScene* TEMP_scene;
	glm::mat4 globalInverseTransform;
	//for fast retrieval of a channel by name (once we copy to our own data struct we can optimize that)
	std::map<unsigned long, aiNodeAnim*> nodeAnimLookup;

	//TODO : move that to an animation class, temp test
	double animationTime;

	GLuint boneTransformBuffer;
	GLuint boneTransformBlockIndex;
};

namespace rendertarget
{
	void init(RenderTarget* rt, int w, int h);
	void bind(RenderTarget* rt);
}

namespace mesh
{
	void fill(Mesh* mesh, Vertex* vertex, int vertexCount, unsigned int* indices, int indicesCount);
	void render(Mesh* mesh, Material* mat, glm::mat4 modelMatrix, glm::mat4 camera);
}

namespace model
{
	void fromFile(Model* m, const char* file);
	void tickAnimation(Model* m, float deltaTime);
	void render(Model* m, Camera* cam);
}