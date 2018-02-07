#include "animation.h"
#include "graphic.h"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

//Hash function to hash bone name to use as ID in the bone lookup function
uint64_t hashString(const char *str)
{
	uint64_t hash = 0;
	int c;

	while (c = *str++)
		hash = c + (hash << 6) + (hash << 16) - hash;

	return hash;
}

glm::quat createQuat(const aiQuaternion& quat)
{
	return glm::quat(quat.w, quat.x, quat.y, quat.z);
}


glm::vec3 animkeyvec::interpolate(AnimKeyVec* keys, int keyCount, double timeInTick)
{
	if (keyCount == 1)
		return keys[0].value;

	int startIdx = 0;
	double startTime = 0;
	double endTime = 0;

	for (int i = 1; i < keyCount - 1; ++i)
	{
		float t = keys[i].time;
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

	return glm::mix(keys[startIdx].value, keys[startIdx+1].value, glm::clamp(amount, 0.0, 1.0));
}

glm::quat animkeyquat::interpolate(AnimKeyQuat* keys, int keyCount, double timeInTick)
{
	if (keyCount == 1)
	{
		return keys[0].value;
	}

	int startIdx = 0;
	double startTime = 0;
	double endTime = 0;

	for (int i = 1; i < keyCount - 1; ++i)
	{
		float t = keys[i].time;
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

	return glm::mix(keys[startIdx].value, keys[startIdx+1].value, (float)glm::clamp(amount, 0.0, 1.0));
}


//----------- animation -----------------

void animation::fromScene(std::vector<Animation>& animationsList, const aiScene* scene)
{
	int animCount = scene->mNumAnimations;
	for (int i = 0; i < animCount; ++i)
	{
		aiAnimation* aiAnim = scene->mAnimations[i];
		Animation anim;

		int nameLen = strlen(aiAnim->mName.data) + 1;
		nameLen = std::min(1024, nameLen);
		strcpy_s(anim.name, nameLen, aiAnim->mName.data);

		anim.tickDuration = aiAnim->mDuration;
		anim.tickPerSecond = aiAnim->mTicksPerSecond;

		int numChannels = aiAnim->mNumChannels;
		anim.animatioNodes = new NodeAnimation[numChannels];

		for (int k = 0; k < numChannels; ++k)
		{
			aiNodeAnim* animNode = aiAnim->mChannels[k];
			anim.animatioNodes[k].nameHash = hashString(animNode->mNodeName.data);

			anim.animatioNodes[k].positionKeyCount = animNode->mNumPositionKeys;
			anim.animatioNodes[k].positionKeys = new AnimKeyVec[animNode->mNumPositionKeys];
			for (int n = 0; n < animNode->mNumPositionKeys; ++n)
			{
				anim.animatioNodes[k].positionKeys[n].time = animNode->mPositionKeys[n].mTime;
				anim.animatioNodes[k].positionKeys[n].value = glm::make_vec3(&animNode->mPositionKeys[n].mValue.x);
			}

			anim.animatioNodes[k].scaleKeyCount = animNode->mNumScalingKeys;
			anim.animatioNodes[k].scaleKeys = new AnimKeyVec[animNode->mNumScalingKeys];
			for (int n = 0; n < animNode->mNumScalingKeys; ++n)
			{
				anim.animatioNodes[k].scaleKeys[n].time = animNode->mScalingKeys[n].mTime;
				anim.animatioNodes[k].scaleKeys[n].value = glm::make_vec3(&animNode->mScalingKeys[n].mValue.x);
			}

			anim.animatioNodes[k].rotationKeyCount = animNode->mNumRotationKeys;
			anim.animatioNodes[k].rotationKeys = new AnimKeyQuat[animNode->mNumRotationKeys];
			for (int n = 0; n < animNode->mNumRotationKeys; ++n)
			{
				anim.animatioNodes[k].rotationKeys[n].time = animNode->mRotationKeys[n].mTime;
				anim.animatioNodes[k].rotationKeys[n].value = createQuat(animNode->mRotationKeys[n].mValue);
			}

			anim.nodeAnimationLookup[anim.animatioNodes[k].nameHash] = k;
		}

		animationsList.push_back(anim);
	}
}

void animation::free(Animation anim)
{
	delete anim.animatioNodes;
}


void tickNodeHierarchy(Model* m, Animation* animation, int sceneNode, double timeInTick, const glm::mat4 parentTransform)
{
	SceneNode* node = &m->sceneHierarchy[sceneNode];
	glm::mat4 transformation = node->transform;

	auto nodeAnimIt = animation->nodeAnimationLookup.find(node->nameHash);

	if (nodeAnimIt != animation->nodeAnimationLookup.end())
	{
		NodeAnimation* n = &animation->animatioNodes[nodeAnimIt->second];

		glm::mat4 scale = glm::scale(animkeyvec::interpolate(n->scaleKeys, n->scaleKeyCount, timeInTick));
		glm::mat4 rotation = glm::toMat4(animkeyquat::interpolate(n->rotationKeys, n->rotationKeyCount, timeInTick));
		glm::mat4 translation = glm::translate(animkeyvec::interpolate(n->positionKeys, n->positionKeyCount, timeInTick));

		transformation = translation * rotation * scale;
	}

	glm::mat4 globalTransform = parentTransform * transformation;

	auto boneIt = m->nodeMapping.find(node->nameHash);
	if (boneIt != m->nodeMapping.end())
	{
		m->boneTransform[boneIt->second] = m->globalInverseTransform * globalTransform * m->boneOffsets[boneIt->second];
	}

	for (int i = 0; i < node->childrenCount; i++)
	{
		tickNodeHierarchy(m, animation, node->children[i], timeInTick, globalTransform);
	}
}

//return new time
float  animation::tickAnimation(Model* m, Animation* anim, float currentTime, float deltaTime)
{
	double tickPerSecond = anim->tickPerSecond == 0.0 ? 24.0 : anim->tickPerSecond;

	currentTime += (deltaTime * tickPerSecond);
	currentTime = fmod(currentTime, anim->tickDuration);

	tickNodeHierarchy(m, anim, 0, currentTime, glm::mat4(1.0f));

	glBindBuffer(GL_UNIFORM_BUFFER, m->boneTransformBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4) * m->boneCount, glm::value_ptr(m->boneTransform[0]));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	return currentTime;
}