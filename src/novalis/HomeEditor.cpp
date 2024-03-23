#include "HomeEditor.h"

nv::editor::EditorDest nv::editor::runHomeEditor(Renderer& renderer) {
	static constexpr ImVec2 windowPos{ 1000.0f, 500.0f };
	static constexpr ImVec2 windowSize{ 200.0f, 200.0f };
	static constexpr ImVec2 buttonSize{ 183.0f, 80.0f };

	ImGui::SetNextWindowPos(windowPos);
	ImGui::SetNextWindowSize(windowSize);
	ImGui::Begin("Options");
	ScopeExit exit{ &ImGui::End };
	if (ImGui::Button("Sprite Editor", buttonSize)) {
		return EditorDest::Sprite;
	}
	if (ImGui::Button("Scene editor", buttonSize)) {
		return EditorDest::Scene;
	}
	return EditorDest::None;
}