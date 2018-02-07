#pragma once

#include <map>
#include <vector>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

#include <assimp/scene.h>
#include <stdint.h>

struct Model;

struct AnimKeyVec
{
	glm::vec3 value;
	double time;
};

namespace animkeyvec
{
	glm::vec3 interpolate(AnimKeyVec* keys, int keyCount, double timeInTick);
}

struct AnimKeyQuat
{
	glm::quat value;
	double time;
};

namespace animkeyquat
{
	glm::quat interpolate(AnimKeyQuat* keys, int keyCount, double timeInTick);
}

//This is animation for a given node (matched by name)
struct NodeAnimation
{
	uint64_t nameHash;

	int positionKeyCount;
	AnimKeyVec* positionKeys;

	int rotationKeyCount;
	AnimKeyQuat* rotationKeys;

	int scaleKeyCount;
	AnimKeyVec* scaleKeys;
};

struct Animation
{
	char name[1024];

	double tickPerSecond;
	double tickDuration;

	NodeAnimation* animatioNodes;
	std::map<unsigned int, int> nodeAnimationLookup;
};

namespace animation
{
	void fromScene(std::vector<Animation>& animationsList, const aiScene* scene);
	float tickAnimation(Model* m, Animation* anim, float currentTime, float deltaTime);

	//we have dynamic memory used in the animation struct, so use that to clean properly
	//(don't use destructor so we can pass the structure around even by copy easily and control its lifetime manually)
	void free(Animation anim);
}

//Hash function to hash bone name to use as ID in the bone lookup function
uint64_t hashString(const char *str);
glm::quat createQuat(const aiQuaternion& quat);