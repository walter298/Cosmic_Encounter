#include "HomeEditor.h"

void nv::editor::runHomeEditor(EditorDest& dest, bool& running) {
	constexpr ImVec2 buttonSize{ 100.0f, 50.0f };

	ImGui::Begin("Options");
	if (ImGui::Button("Object Editor")) {
		dest = EditorDest::Object;
		running = false;
	}
	if (ImGui::Button("Text editor", buttonSize)) {
		dest = EditorDest::Text;
		running = false;
	}
	ImGui::End();
}