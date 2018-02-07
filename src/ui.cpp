#include "ui.h"
#include <cstdio>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

const aiScene* gCurrentOpenedScene = nullptr;
char* gPathToOpen = nullptr;

void DrawModelImport(Model* model, std::vector<Animation>& animList)
{
	if (gPathToOpen != nullptr)
	{
		gCurrentOpenedScene = aiImportFile(gPathToOpen, aiProcessPreset_TargetRealtime_MaxQuality);
		delete gPathToOpen;
		gPathToOpen = nullptr;

		ImGui::OpenPopup("Import");
	}

	if (ImGui::BeginPopupModal("Import", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
	{
		bool meshPresent = gCurrentOpenedScene->mNumMeshes > 0;
		bool animPresent = gCurrentOpenedScene->mNumAnimations > 0;



		if (meshPresent && ImGui::Button("Import Mesh"))
		{
			model::fromScene(model, gCurrentOpenedScene);

			aiReleaseImport(gCurrentOpenedScene);
			gCurrentOpenedScene = nullptr;
			ImGui::CloseCurrentPopup();
		}

		if (model->meshArray == nullptr && !meshPresent && animPresent)
		{
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "You can't load animations before any mesh, load a mesh first, and this file contains none.");
		}
		else if(model->meshArray != nullptr && animPresent && ImGui::Button("Import Animations"))
		{
			animation::fromScene(animList, gCurrentOpenedScene);

			aiReleaseImport(gCurrentOpenedScene);
			gCurrentOpenedScene = nullptr;
			ImGui::CloseCurrentPopup();
		}

		if (meshPresent && animPresent && ImGui::Button("Import Mesh And Animations"))
		{
			model::fromScene(model, gCurrentOpenedScene);
			animation::fromScene(animList, gCurrentOpenedScene);

			aiReleaseImport(gCurrentOpenedScene);
			gCurrentOpenedScene = nullptr;
			ImGui::CloseCurrentPopup();
		}

		if (!meshPresent && !animPresent)
		{
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "That file does not contains any meshes or animations.");
			if (ImGui::Button("Close"))
			{
				ImGui::CloseCurrentPopup();
			}
		}
		else if (ImGui::Button("Cancel"))
		{
			aiReleaseImport(gCurrentOpenedScene);
			gCurrentOpenedScene = nullptr;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

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

bool getAnimationName(void* data, int i, const char** text)
{
	std::vector<Animation>* animationList = (std::vector<Animation>*)data;

	*text = animationList->at(i).name;

	return true;
}

void DrawAnimationList(AnimationListState* state, std::vector<Animation>& animationList)
{
	ImGui::Begin("Animation List");

	ImGui::ListBox("Anim list", &state->selectedAnimation, getAnimationName, &animationList, (int)animationList.size());

	ImGui::End();
}

void fileDropCallback(GLFWwindow* window, int count, const char** paths)
{
	if (count > 1)
	{
		printf("[Error] Application don't handle opening more than 1 file at the time for now");
		return;
	}

	if (gPathToOpen != nullptr)
		delete gPathToOpen;

	int len = strlen(paths[0])+1;
	len = std::min(4096, len);
	gPathToOpen = new char[len];
	strcpy_s(gPathToOpen, len, paths[0]);
}