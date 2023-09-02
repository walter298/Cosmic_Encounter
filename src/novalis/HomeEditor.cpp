#include "HomeEditor.h"

using nv::editor::HomeEditor;

HomeEditor::HomeEditor(ImGuiIO& io, SDL_Renderer* renderer)
	: Editor(io, renderer) {}

nv::editor::EditorDest HomeEditor::getDestination() const noexcept {
	return m_dest;
}

void HomeEditor::showGui(bool& running) {
	constexpr ImVec2 windowPos{ 1000.0f, 500.0f };
	constexpr ImVec2 windowSize{ 200.0f, 200.0f };
	constexpr ImVec2 buttonSize{ 183.0f, 80.0f };

	ImGui::SetNextWindowPos(windowPos);
	ImGui::SetNextWindowSize(windowSize);
	ImGui::Begin("Options");
	if (ImGui::Button("Sprite Editor", buttonSize)) {
		m_dest = EditorDest::Sprite;
		running = false;
	}
	if (ImGui::Button("Text editor", buttonSize)) {
		m_dest = EditorDest::Text;
		running = false;
	}
	ImGui::End();
}