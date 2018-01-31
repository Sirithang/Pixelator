#pragma once

#include "imgui.h"
#include "graphic.h"

struct MainMenuState
{
	bool fileEntrySelected;
};

struct SpriteViewerData
{
	RenderTarget* associatedRT;

	glm::vec3 cameraPosition;
	glm::vec3 cameraTarget;
};

void DrawMainMenu(MainMenuState* state);
void DrawSpriteViewer(SpriteViewerData* data);