#include "SpriteEditor.h"

using nv::editor::SpriteEditor;

void SpriteEditor::writeImagePathsAndSaveToFile(const std::string& savedPath, nlohmann::json& j) {
	j["texture_paths"] = std::vector<std::string>();
	for (auto& path : m_texturePaths) {
		std::ranges::replace(path, '\\', '/');
		j.at("texture_paths").push_back(path);
	}

	std::ofstream spriteFile{ savedPath };
	assert(spriteFile.is_open());
	spriteFile << j.dump(2);
	spriteFile.close();
}

void SpriteEditor::showSpriteOptions(EditorRenderer& renderer) {
	static constexpr ImVec2 spriteOptionsPos{ spriteBoxPos.x, spriteBoxPos.y + spriteBoxSize.y - 70.0f };
	static constexpr ImVec2 spriteOptionsSize{ 300.0f, 250.0f };
	static constexpr ImVec2 btnSize{ 270.0f, 50.0f };
	
	ImGui::SetNextWindowPos(spriteOptionsPos);
	ImGui::SetNextWindowSize(spriteOptionsSize);
	ImGui::Begin("Sprite Options");
	m_spriteRectEditor.edit(false);
	if (ImGui::Button("Save Sprite", btnSize)) {
		auto savedPath = saveFile(L"Save Sprite");
		if (savedPath) {
			json j;
			j["ren"] = m_sprite.ren;
			writeImagePathsAndSaveToFile(*savedPath, j);
		}
	}

	const bool prevNoSprites = m_sprite.m_textures.empty();

	if (ImGui::Button("Upload Image(s)", btnSize)) {
		loadImages(m_texturePaths, m_textures, renderer);
	}
	
	if (prevNoSprites && !m_sprite.m_textures.empty()) {
		renderer.addObj(&m_sprite, 0);
	}
	ImGui::End();
}

void SpriteEditor::selectSprite(EditorRenderer& renderer) {
	static constexpr ImVec2 btnSize{ 270.0f, 50.0f };

	ImGui::SetNextWindowPos(spriteSwitchPos);
	ImGui::SetNextWindowSize(spriteSwitchSize);
	ImGui::Begin("Sprite Sheet");

	bool deleteSpriteInput = false;

	size_t doomedTexIdx = 0;

	for (const auto [idx, texture] : std::views::enumerate(m_textures)) {
		ImGui::PushID(idx);

		if (ImGui::ImageButton(ImTextureID(m_xBtn.raw), xBtnSize)) {
			deleteSpriteInput = true;
			doomedTexIdx = idx;
		}
		ImGui::PopID();

		auto texID = m_textures[idx]->raw;
		ImGui::PushID(texID);

		ImGui::SameLine(70.0f); //show textures on the same line as the x-button

		if (ImGui::ImageButton(ImTextureID(texID), spriteBtnSize) && !deleteSpriteInput) {
			m_sprite.changeTexture(idx);
		}
		ImGui::PopID();
	}
	
	const bool prevHadSprites = !m_sprite.m_textures.empty();
	if (deleteSpriteInput) {
		m_textures.erase(m_textures.begin() + doomedTexIdx);
		if (m_textures.empty() && prevHadSprites) {
			renderer.removeObj(m_sprite.getID(), 0);
		} else if (m_sprite.m_currTexGroupIdx == m_textures.size()) {
			m_sprite.m_currTexGroupIdx--;
		}
	}
	
	ImGui::End();
}

nv::editor::SpriteEditor::SpriteEditor(EditorRenderer& renderer) noexcept 
	: m_xBtn{ IMG_LoadTexture(renderer.get(), relativePath("Cosmic_Encounter/src/novalis/assets/x_btn.png").c_str()) }
{
	m_spriteRectEditor.rect = &m_sprite.ren;
}

nv::editor::EditorDest SpriteEditor::operator()(EditorRenderer& renderer) {
	showSpriteOptions(renderer);
	selectSprite(renderer);
	return EditorDest::None;
}