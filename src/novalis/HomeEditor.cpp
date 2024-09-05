#include "HomeEditor.h"

#include "data_util/DataStructures.h" //ScopeExit

nv::editor::EditorDest nv::editor::HomeEditor::imguiRender() noexcept {
	static constexpr ImVec2 winPos{ NV_SCREEN_WIDTH / 2, NV_SCREEN_HEIGHT / 2 };
	static constexpr ImVec2 btnSize{ 183.0f, 80.0f };
	static constexpr ImVec2 winSize = buttonList(btnSize, 2);

	ImGui::SetNextWindowPos(winPos);
	ImGui::SetNextWindowSize(winSize);
	ImGui::Begin("Options");
	ScopeExit exit{ &ImGui::End };
	if (ImGui::Button("Sprite Editor", btnSize)) {
		return EditorDest::Sprite;
	}
	if (ImGui::Button("Scene Editor", btnSize)) {
		return EditorDest::Scene;
	}
	return EditorDest::None;
}
