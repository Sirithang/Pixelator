#pragma once

#include "imgui.h"
#include "graphic.h"
#include "animation.h"

#include <vector>

struct MainMenuState
{
	bool fileEntrySelected;
};

struct SpriteViewerData
{
	RenderTarget* associatedRT;
	Camera* camera;
};

struct AnimationListState
{
	int selectedAnimation;
};

void DrawModelImport(Model* model, std::vector<Animation>& animList);

void DrawMainMenu(MainMenuState* state);
void DrawSpriteViewer(SpriteViewerData* data);
void DrawAnimationList(AnimationListState* state, std::vector<Animation>& animationList);

void fileDropCallback(GLFWwindow* window, int count, const char** paths);

extern const aiScene* gCurrentOpenedScene;
extern char* gPathToOpen;