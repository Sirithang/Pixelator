#include <graphic.h>
#include <cstdio>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

const char *vertexShaderSource =
"#version 330 core\n"
"\n"
"// Input vertex data, different for all executions of this shader.\n"
"layout(location = 0) in vec3 vertexPosition_modelspace;\n"
"layout(location = 1) in vec3 vertexNomal;\n"
"layout(location = 2) in vec3 vertexUV;\n"
"\n"
"// Output data ; will be interpolated for each fragment.\n"
"out vec3 texcoord;\n"
"out vec3 normal;\n"
"\n"
"// Values that stay constant for the whole mesh.\n"
"uniform mat4 M;\n"
"uniform mat4 MVP;\n"
"\n"
"void main(){\t\n"
"\n"
"\t// Output position of the vertex, in clip space : MVP * position\n"
"\tgl_Position =  MVP * vec4(vertexPosition_modelspace,1);\n"
"\n"
"\t// The color of each vertex will be interpolated\n"
"\t// to produce the color of each fragment\n"
"\tnormal = vertexNomal;\n"
"\ttexcoord = vertexUV;\n"
"}";


const char *fragmentShaderSource = 
"#version 330 core\n"
"\n"
"// Interpolated values from the vertex shaders\n"
"in vec3 texcoord;\n"
"in vec3 normal;\n"
"\n"
"// Ouput data\n"
"out vec3 color;\n"
"\n"
"void main(){\n"
"\n"
"\tvec3 col = vec3(1,1,1) * dot(normalize(vec3(0,0,1)), normal);\n"
"\t// Output color = color specified in the vertex shader, \n"
"\t// interpolated between all 3 surrounding vertices\n"
"\tcolor = col;\n"
"\n"
"}";

//TODO : move to utility h/cpp?
//Hash function to hash bone name to use as ID in the bone lookup function
static unsigned long
sdbm(const char *str)
{
	unsigned long hash = 0;
	int c;

	while (c = *str++)
		hash = c + (hash << 6) + (hash << 16) - hash;

	return hash;
}

GLuint defaultProgram = -1;

GLuint LoadDefaultShaders()
{
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	GLint Result = GL_FALSE;
	int InfoLogLength;


	char *buffer;

	//reading file
	FILE *f;
	fopen_s(&f, "testData/vertex.vs", "rb");
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);  //same as rewind(f);

	buffer = new char[fsize + 1];
	fread(buffer, fsize, 1, f);
	fclose(f);

	buffer[fsize] = 0;

	// Compile Vertex Shader
	printf("Compiling vertex shader\n");
	glShaderSource(VertexShaderID, 1, &buffer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) 
	{
		char* VertexShaderErrorMessage = new char[InfoLogLength + 1];
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	//reading file
	fopen_s(&f, "testData/pixel.ps", "rb");
	fseek(f, 0, SEEK_END);
	fsize = ftell(f);
	fseek(f, 0, SEEK_SET);  //same as rewind(f);

	buffer = new char[fsize + 1];
	fread(buffer, fsize, 1, f);
	fclose(f);

	buffer[fsize] = 0;

	glShaderSource(FragmentShaderID, 1, &buffer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		char* FragmentShaderErrorMessage = new char[InfoLogLength + 1];
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}



	// Link the program
	printf("Linking program\n");


	GLuint ProgramID;
	
	if (defaultProgram == -1)
	{
		ProgramID = glCreateProgram();
	}
	else
	{
		ProgramID = defaultProgram;
	}
	
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		char* ProgramErrorMessage = new char[InfoLogLength + 1];
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}


	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	defaultProgram = ProgramID;
	return ProgramID;
}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	else if (key == GLFW_KEY_F5 && action == GLFW_PRESS)
		LoadDefaultShaders();
}

GLFWwindow* CreateWindow()
{
	GLFWwindow* window;

	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
		return NULL;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, "Pixelator", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return NULL;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	glewInit();

	glEnable(GL_DEPTH);

	return window;
}

void rendertarget::init(RenderTarget* rt, int w, int h)
{
	rt->w = w;
	rt->h = h;

	glGenFramebuffers(1, &rt->framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, rt->framebuffer);

	// The texture we're going to render to
	glGenTextures(1, &rt->texture);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, rt->texture);

	// Give an empty image to OpenGL ( the last "0" )
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	// Poor filtering. Needed !
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	// The depth buffer
	glGenRenderbuffers(1, &rt->depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, rt->depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rt->depthBuffer);

	// Set "renderedTexture" as our colour attachement #0
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, rt->texture, 0);

	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers
}

void rendertarget::bind(RenderTarget* rt)
{
	glBindFramebuffer(GL_FRAMEBUFFER, rt->framebuffer);
	glViewport(0, 0, rt->w, rt->h); // Render on the whole framebuffer, complete from the lower left corner to the upper right
}

void mesh::fill(Mesh* mesh, Vertex* vertex, int vertexCount, unsigned int* indices, int indicesCount)
{
	glGenBuffers(1, &mesh->vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertexCount, vertex, GL_STATIC_DRAW);

	glGenBuffers(1, &mesh->indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indicesCount, indices, GL_STATIC_DRAW);

	mesh->indexCount = indicesCount;
}


void mesh::render(Mesh* mesh, Material* mat, glm::mat4 modelMatrix, glm::mat4 camera)
{

	// Our ModelViewProjection : multiplication of our 3 matrices
	glm::mat4 MVP = camera * modelMatrix; // Remember, matrix multiplication is the other way around

	// Use our shader
	glUseProgram(mat->program);

	// Send our transformation to the currently bound shader, 
	// in the "MVP" uniform
	glUniformMatrix4fv(mat->MVPLocation, 1, GL_FALSE, &MVP[0][0]);

	// Send our transformation to the currently bound shader, 
	// in the "MVP" uniform
	glUniformMatrix4fv(mat->MLocation, 1, GL_FALSE, &modelMatrix[0][0]);


	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);

	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
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

	glVertexAttribIPointer(
		3,                  // attribute. No particular reason for 0, but must match the layout in the shader.
		4,                  // size
		GL_INT,           // type
		sizeof(Vertex),                  // stride
		(GLvoid*)(3 * sizeof(glm::vec3))            // array buffer offset
	);

	glVertexAttribPointer(
		4,                  // attribute. No particular reason for 0, but must match the layout in the shader.
		4,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		sizeof(Vertex),                  // stride
		(GLvoid*)(3 * sizeof(glm::vec3) + sizeof(glm::ivec4))            // array buffer offset
	);

	// Draw the triangle !
	//glDrawArrays(GL_TRIANGLES, 0, scene->mMeshes[0]->mNumVertices); // 3 indices starting at 0 -> 1 triangle

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);

	//Draw the triangles !
	glDrawElements(
		GL_TRIANGLES,      // mode
		mesh->indexCount,    // count
		GL_UNSIGNED_INT,   // type
		(void*)0           // element array buffer offset
	);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(4);
}


void model::fromFile(Model* m, const char* file)
{
	//option import generate loads of stuff like tangent, normal etc.. so let's assume they are here
	const aiScene* scene = aiImportFile(file, aiProcessPreset_TargetRealtime_MaxQuality);

	m->TEMP_scene = scene;
	m->globalInverseTransform = glm::inverse(glm::transpose(glm::make_mat4(&scene->mRootNode->mTransformation.a1)));

	aiAnimation* anim = scene->mAnimations[0];
	for (int i = 0; i < anim->mNumChannels; ++i)
	{
		unsigned long hash = sdbm(anim->mChannels[i]->mNodeName.C_Str());

		m->nodeAnimLookup[hash] = anim->mChannels[i];
	}

	m->animationTime = 0;

	m->meshCount = scene->mNumMeshes;

	m->meshArray = new Mesh[m->meshCount];
	m->materialArray = new Material[m->meshCount];
	
	m->boneCount = 0;

	for (int i = 0; i < m->meshCount; ++i)
	{
		aiMesh* mesh = scene->mMeshes[i];
		int numVertex = mesh->mNumVertices;
		int numFaces = mesh->mNumFaces;

		int boneCount = mesh->mNumBones;

		Vertex* vertex = new Vertex[numVertex];
		unsigned int* indices = new unsigned int[numFaces * 3];

		for (int j = 0; j < numVertex; ++j)
		{
			vertex[j].position = *((glm::vec3*)(&mesh->mVertices[j]));
			vertex[j].normal = *((glm::vec3*)(&mesh->mNormals[j]));
			vertex[j].uv = *((glm::vec3*)(&mesh->mTextureCoords[0][j]));

			vertex[j].bonesID = glm::ivec4(0, 0, 0, 0);
			vertex[j].weights = glm::vec4(0, 0, 0, 0);
		}

		for (int j = 0; j < numFaces; ++j)
		{
			indices[j * 3 + 0] = mesh->mFaces[j].mIndices[0];
			indices[j * 3 + 1] = mesh->mFaces[j].mIndices[1];
			indices[j * 3 + 2] = mesh->mFaces[j].mIndices[2];
		}

		for (int j = 0; j < boneCount; ++j)
		{
			aiBone* bone = mesh->mBones[j];

			//find this bone id in the mapping, or if still not mapped, add it
			int mappedIndex = 0;
			unsigned long hash = sdbm(bone->mName.C_Str());
			auto it = m->boneMapping.find(hash);
			if (it == m->boneMapping.end())
			{
				mappedIndex = m->boneCount;
				m->boneCount += 1;
				m->boneMapping[hash] = mappedIndex;
				m->boneOffsets[mappedIndex] = *((glm::mat4*)&bone->mOffsetMatrix.a1);
				m->boneOffsets[mappedIndex] = glm::transpose(m->boneOffsets[mappedIndex]);
			}
			else
				mappedIndex = it->second;
			
			for (int k = 0; k < bone->mNumWeights; ++k)
			{
				Vertex* v = &vertex[bone->mWeights[k].mVertexId];

				int freeIdx = 0;
				while (freeIdx < 4 && v->weights[freeIdx] != 0.0f)
					freeIdx++;

				//we don't go over 4 weight by vertex, if that vertex already have 4 weight, skip it
				//TODO : handle that better, discard always the lowest weight value
				if (freeIdx < 4)
				{
					v->bonesID[freeIdx] = mappedIndex;
					v->weights[freeIdx] = bone->mWeights[k].mWeight;
				}
			}
		}

		mesh::fill(&m->meshArray[i], vertex, numVertex, indices, numFaces * 3);

		m->materialArray[i].program = defaultProgram;
		m->materialArray[i].MVPLocation = glGetUniformLocation(defaultProgram, "MVP");
		m->materialArray[i].MLocation = glGetUniformLocation(defaultProgram, "M");
	}

	GLuint blockIdx = glGetUniformBlockIndex(defaultProgram, "Bone");
	glUniformBlockBinding(defaultProgram, blockIdx, 1);

	glGenBuffers(1, &m->boneTransformBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, m->boneTransformBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * m->boneCount, NULL, GL_STREAM_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, m->boneTransformBuffer);
}

void model::render(Model* m, Camera* cam)
{
	/*glUniformBlockBinding(defaultProgram, 1, 1);
	glBindBufferRange(GL_UNIFORM_BUFFER, 1, m->boneTransformBuffer, 0, sizeof(glm::mat4) * m->boneCount);*/

	for (int i = 0; i < m->meshCount; ++i)
	{
		glm::mat4 Model = glm::mat4(1.0f);
		//Model = glm::translate(Model, glm::vec3(1, 2, 3));
		//Model = glm::translate(Model, glm::vec3(i, 0, 0));
		//Model = glm::scale(Model, glm::vec3(0.1, 0.1, 0.1));

		mesh::render(&m->meshArray[i], &m->materialArray[i], Model, cam->viewProjMat);
	}
}

glm::vec3 interpolateVectorKey(const aiVectorKey* keys, int keyCount, double timeInTick)
{
	if (keyCount == 1)
		return glm::make_vec3(&keys[0].mValue.x);

	int startIdx = 0;
	double startTime = 0;
	double endTime = 0;

	for (int i = 1; i < keyCount-1; ++i)
	{
		float t = keys[i].mTime;
		if (t > timeInTick)
		{
			endTime = t;
			break;
		}

		startTime = t;
		startIdx = i;
	}

	assert(startIdx + 1 < keyCount);

	double amount = (endTime - timeInTick) / (endTime - startTime);

	glm::vec3 start = glm::make_vec3(&keys[startIdx].mValue.x);
	glm::vec3 end = glm::make_vec3(&keys[startIdx + 1].mValue.x);

	return glm::mix(start, end, glm::clamp(amount, 0.0, 1.0));
}

glm::quat createQuat(const aiQuaternion& quat)
{
	return glm::quat(quat.w,quat.x, quat.y,quat.z);
}

glm::quat interpolateQuatKey(const aiQuatKey* keys, int keyCount, double timeInTick)
{
	if (keyCount == 1)
	{
		return createQuat(keys[0].mValue);
	}

	int startIdx = 0;
	double startTime = 0;
	double endTime = 0;

	for (int i = 1; i < keyCount-1; ++i)
	{
		float t = keys[i].mTime;
		if (t > timeInTick)
		{
			endTime = t;
			break;
		}

		startTime = t;
		startIdx = i;
	}

	assert(startIdx + 1 < keyCount);

	double amount = (endTime - timeInTick) / (endTime - startTime);

	glm::quat start = createQuat(keys[startIdx].mValue);
	glm::quat end = createQuat(keys[startIdx + 1].mValue);

	return glm::mix(start, end,(float)glm::clamp(amount, 0.0, 1.0));
}

void tickNodeHierarchy(Model* m, const aiAnimation* animation, double timeInTick, const aiNode* pNode, const glm::mat4 parentTransform)
{
	unsigned long nameHash = sdbm(pNode->mName.C_Str());

	glm::mat4 transformation = glm::transpose(glm::make_mat4(&pNode->mTransformation.a1));

	const aiNodeAnim* pNodeAnim = m->nodeAnimLookup[nameHash];
	auto it = m->boneMapping.find(nameHash);

	if (pNodeAnim) 
	{
		glm::mat4 scale = glm::scale(interpolateVectorKey(pNodeAnim->mScalingKeys, pNodeAnim->mNumScalingKeys, timeInTick));
		glm::mat4 rotation = glm::toMat4(interpolateQuatKey(pNodeAnim->mRotationKeys, pNodeAnim->mNumRotationKeys, timeInTick));
		glm::mat4 translation = glm::translate(interpolateVectorKey(pNodeAnim->mPositionKeys, pNodeAnim->mNumPositionKeys, timeInTick));

		transformation =  translation * rotation * scale;
	}

	glm::mat4 globalTransform = parentTransform * transformation;

	if (it != m->boneMapping.end())
	{
		m->boneTransform[it->second] = m->globalInverseTransform * globalTransform * m->boneOffsets[it->second];
	}

	for (int i = 0; i < pNode->mNumChildren; i++) 
	{
		tickNodeHierarchy(m, animation, timeInTick, pNode->mChildren[i], globalTransform);
	}
}

void  model::tickAnimation(Model* m, float deltaTime)
{
	aiAnimation* anim = m->TEMP_scene->mAnimations[0];
	double tickPerSecond = anim->mTicksPerSecond == 0.0 ? 24.0 : anim->mTicksPerSecond;

	m->animationTime += (deltaTime * tickPerSecond);
	m->animationTime = fmod(m->animationTime, anim->mDuration);

	tickNodeHierarchy(m, anim, m->animationTime, m->TEMP_scene->mRootNode, glm::mat4(1.0f));

	glBindBuffer(GL_UNIFORM_BUFFER, m->boneTransformBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4) * m->boneCount, glm::value_ptr(m->boneTransform[0]));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}