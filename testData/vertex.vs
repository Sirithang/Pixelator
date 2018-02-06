#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexNomal;
layout(location = 2) in vec3 vertexUV;
layout(location = 3) in ivec4 boneIDs;
layout(location = 4) in vec4 weights;

// Output data ; will be interpolated for each fragment.
out vec3 texcoord;
out vec3 normal;
out vec3 firstBoneWeights;

const int MAX_BONES = 70;

// Values that stay constant for the whole mesh.
uniform mat4 M;
uniform mat4 MVP;

layout(std140) uniform Bone
{
  mat4 bones[MAX_BONES];
};

void main()
{
    vec4 vert =  vec4(vertexPosition_modelspace, 1.0);
    //vec4 PosL = (bones[boneIDs[0]] * vert) * weights[0] + 
	//(bones[boneIDs[1]] * vert) * weights[1] + 
	//(bones[boneIDs[2]] * vert) * weights[2] + 
	//(bones[boneIDs[3]] * vert) * weights[3];

	vec4 PosL = 
	(bones[boneIDs[0]] * vert) * weights[0] + 
	(bones[boneIDs[1]] * vert) * weights[1] + 
	(bones[boneIDs[2]] * vert) * weights[2] + 
	(bones[boneIDs[3]] * vert) * weights[3];

    //vec4 PosL = BoneTransform * vec4(vertexPosition_modelspace, 1.0);
	// Output position of the vertex, in clip space : MVP * position
	gl_Position =  MVP * PosL;

	// The color of each vertex will be interpolated
	// to produce the color of each fragment
	normal = vertexNomal;
	texcoord = vertexUV;

	firstBoneWeights = vec3(1.0f,0,0);
}