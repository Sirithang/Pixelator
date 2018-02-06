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

	Camera* camera;
};

void DrawMainMenu(MainMenuState* state);
void DrawSpriteViewer(SpriteViewerData* data);