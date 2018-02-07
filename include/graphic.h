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

//Full scene hierarchy
struct SceneNode
{
	glm::mat4 transform;

	//for our purpose, no need to keep bones names, we just store the hash used for lookup
	uint64_t nameHash;

	int parent;

	int children[32];
	int childrenCount;
};

struct Model
{
	int meshCount;

	Mesh* meshArray;
	Material* materialArray;

	//map the node in the loaded model to their index into the node array.
	//useful to get which index in the bone array a specific vertex is bound to
	std::map<unsigned int, int> nodeMapping;

	//let's use a static array for now, should be enough for our purpose, and take "only" ~200kB
	SceneNode sceneHierarchy[1024];
	int sceneNodeCount;
	
	//hard limit, we only support 100 bones max in GPU anyway.
	//Transform is a separate array to upload it right away to GPU easily
	//TODO make that slightly more dynamic, right now, that take ~7kB no matter the bone count of the model
	//BoneData bonesData[100];
	glm::mat4 boneTransform[100];

	//look like this can be different per mesh for multiple meshes scene, so should probably be stored on the mesh, TODO : investigate
	glm::mat4 boneOffsets[100];
	int boneCount;


	glm::mat4 globalInverseTransform;

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
	void fromScene(Model* m, const aiScene* file);
	void render(Model* m, Camera* cam);
}