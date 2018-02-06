#include "ui.h"
#include <cstdio>

void DrawMainMenu(MainMenuState* state)
{
	ImGui::BeginMainMenuBar();

	ImGui::MenuItem("File", "Alt+F", &state->fileEntrySelected);

	ImGui::EndMainMenuBar();

	ImGuiIO& io = ImGui::GetIO();
	if (io.KeysDown[GLFW_KEY_F5] && io.KeysDownDuration[GLFW_KEY_F5] == 0.0f)
	{
		LoadDefaultShaders();
	}
}

void DrawSpriteViewer(SpriteViewerData* state)
{
	ImGui::Begin("Sprite Render", NULL, ImGuiWindowFlags_NoCollapse);
	
	if (ImGui::CollapsingHeader("Camera Control"))
	{
		ImGui::DragFloat3("Position", &state->camera->position[0]);
		ImGui::DragFloat3("Target", &state->camera->target[0]);
	}

	ImVec2 remainingSpace = ImGui::GetContentRegionAvail();

	float remainingRatio = remainingSpace.x / remainingSpace.y;
	float pixelImageRatio = state->associatedRT->w / state->associatedRT->h;
	
	ImVec2 uv0, uv1;
	if (remainingRatio - pixelImageRatio > 0)
	{
		float ratioWidth = (pixelImageRatio * remainingSpace.y) / remainingSpace.x;
		float wDiff = (1.0f - ratioWidth) * 0.5f / ratioWidth;

		uv0 = ImVec2(-wDiff, 1);
		uv1 = ImVec2(1+wDiff, 0);
	}
	else
	{
		float ratioHeight = ((1.0f/pixelImageRatio) * remainingSpace.x) / remainingSpace.y;
		float hDiff = (1.0f - ratioHeight) * 0.5f / ratioHeight;
		uv0 = ImVec2(0, 1 + hDiff);
		uv1 = ImVec2(1, -hDiff);
	}

	ImGui::Image((ImTextureID)state->associatedRT->texture, remainingSpace, uv0, uv1);
	ImGui::End();
}

void DrawCameraControl()
{

}