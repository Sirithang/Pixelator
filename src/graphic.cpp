#include <graphic.h>
#include <cstdio>

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

GLuint LoadDefaultShaders()
{

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
	printf("Compiling vertex shader\n");
	glShaderSource(VertexShaderID, 1, &vertexShaderSource, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		char* VertexShaderErrorMessage = new char[InfoLogLength + 1];
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}


	glShaderSource(FragmentShaderID, 1, &fragmentShaderSource, NULL);
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
	GLuint ProgramID = glCreateProgram();
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
}

GLFWwindow* CreateWindow()
{
	GLFWwindow* window;

	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
		return NULL;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, "Pixelator", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return NULL;
	}
	glfwSetKeyCallback(window, key_callback);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	glewInit();

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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

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