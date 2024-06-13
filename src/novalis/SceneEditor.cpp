#include "SceneEditor.h"

using nv::editor::SceneEditor;

void SceneEditor::showRightClickOptions(Renderer& renderer, const Coord& mousePos) {
	static constexpr ImVec2 btnSize{ 210.0f, 60.0f };
	static constexpr auto winSize = buttonList(btnSize, 2);

	ImGui::SetNextWindowPos(m_rightClickWinPos);
	ImGui::SetNextWindowSize(winSize);
	ImGui::Begin("Options");

	if (ImGui::Button("Insert Sprite", btnSize)) {
		insertObjFromFile(renderer, m_sprites[m_currLayer]);
		m_showingRightClickOptions = false;
	}
	ImGui::End();
}

void SceneEditor::moveCamera(Renderer& renderer) noexcept {
	static constexpr int CAMERA_DELTA = 15;
	if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
		renderer.move(-CAMERA_DELTA, 0);
		//renderer.moveRects(-CAMERA_DELTA, 0);
	} else if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
		renderer.move(CAMERA_DELTA, 0);
		//renderer.moveRects(CAMERA_DELTA, 0);
	} else if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
		renderer.move(0, CAMERA_DELTA);
		//renderer.moveRects(0, CAMERA_DELTA);
	} else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
		renderer.move(0, -CAMERA_DELTA);
		//renderer.moveRects(0, -CAMERA_DELTA);
	}
}

SceneEditor::SceneEditor(Renderer& renderer) 
	: m_objEditor{ renderer, { 0, 0 } }
{ 
	m_objEditor.reseat(&m_sprites[0], 0);
}

nv::editor::EditorDest SceneEditor::operator()(Renderer& renderer) {
	const auto mousePos = convertPair<Coord>(ImGui::GetMousePos());

	m_objEditor();

	if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
		m_showingRightClickOptions = true;
		m_rightClickWinPos = convertPair<ImVec2>(mousePos);
	}
	if (m_showingRightClickOptions) {
		showRightClickOptions(renderer, mousePos);
	}

	moveCamera(renderer);

	return EditorDest::None;
}