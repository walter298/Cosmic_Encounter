#include "SceneEditor.h"

using nv::editor::SceneEditor;

void SceneEditor::editCurrSprite(Renderer& renderer) {
	auto mousePos = ImGui::GetMousePos();

	//Find which sprites overlap on mouse click
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
		m_overlappingSprites.clear();
		m_selectedSpriteIdx = 0;
		m_showingOptionToInsertSprite = false;
		for (auto& [name, sprites] : m_sprites) {
			for (auto& sprite : sprites) {
				if (sprite.ren.isCoordContained(static_cast<int>(mousePos.x), static_cast<int>(mousePos.y))) {
					m_overlappingSprites.push_back(&sprite);
				}
			}
		} 
	//open option to insert sprite
	} else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
		m_showingOptionToInsertSprite = true;
		m_currSpriteInsertionWindowPos = mousePos;
	}

	static constexpr ImVec2 spriteInsertionSize{ 210.0f, 100.0f };
	static constexpr ImVec2 spriteInsertionBtnSize{ 210.0f, 75.0f };

	if (m_showingOptionToInsertSprite) {
		ImGui::SetNextWindowPos(m_currSpriteInsertionWindowPos);
		ImGui::SetNextWindowSize(spriteInsertionSize);
		ImGui::Begin("Insert Sprite");
		ImGui::Button("Insert Sprite", spriteInsertionBtnSize);
		ImGui::End();
	}
	//show sprite options if sprite is showing
	else if (!m_overlappingSprites.empty()) {
		
	}

	auto [dmx, dmy] = ImGui::GetMouseDragDelta();
}

SceneEditor::SceneEditor() {
	
}

nv::editor::EditorDest SceneEditor::operator()(Renderer& renderer) {
	editCurrSprite(renderer);
	return EditorDest::None;
}