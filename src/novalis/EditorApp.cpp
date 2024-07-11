#include "EditorApp.h"

#include "Instance.h"

void nv::editor::runEditors() {
	Instance instance{ "Novalis" };
	
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	auto& io = ImGui::GetIO();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForSDLRenderer(instance.window, instance.renderer);
	ImGui_ImplSDLRenderer2_Init(instance.renderer);

	bool running = true;
	EditorDest currDest = EditorDest::Home;
	
	while (running) {
		if (currDest == EditorDest::None) {
			continue;
		}
		switch (currDest) {
		case EditorDest::Home:
		{
			HomeEditor homeEditor;
			currDest = runEditor(io, instance.renderer, homeEditor);
			break;
		}
		case EditorDest::Sprite:
		{
			SpriteEditor spriteEditor{ instance.renderer };
			currDest = runEditor(io, instance.renderer, spriteEditor);
			break;
		}
		case EditorDest::Scene:
		{
			SceneEditor sceneEditor{ instance.renderer };
			currDest = runEditor(io, instance.renderer, sceneEditor);
			break;
		}
		case EditorDest::Quit:
			running = false;
			break;
		}
	}
	
	ImGui_ImplSDLRenderer2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}