#include "SpriteEditor.h"

using nv::editor::SpriteEditor;

void SpriteEditor::showSpriteOptions(Renderer& renderer) {
	static constexpr ImVec2 spriteOptionsPos{ spriteBoxPos.x, spriteBoxPos.y + spriteBoxSize.y + 20.0f };
	static constexpr ImVec2 spriteOptionsSize{ 300.0f, 250.0f };
	static constexpr ImVec2 btnSize{ 270.0f, 50.0f };
	
	ImGui::SetNextWindowPos(spriteOptionsPos);
	ImGui::SetNextWindowSize(spriteOptionsSize);
	ImGui::Begin("Sprite Options");
	editRect(m_sprite.ren);
	if (ImGui::Button("Save Sprite", btnSize)) {
		auto savedPath = saveFile(L"Save Sprite");
		if (savedPath) {
			json j;
			j["image_paths"] = std::vector<std::string>();
			j.at("image_paths").get<std::vector<std::string>>().reserve(m_sprite.m_textures.size());
			for (const auto& path : m_texturePaths) {
				j.at("image_paths").push_back(path);
			}
			j["ren"] = m_sprite.ren;
			std::ofstream spriteFile{ savedPath.value() };
			assert(spriteFile.is_open());
			spriteFile << j.dump(2);
			spriteFile.close();
		}
	}

	const bool prevNoSprites = m_sprite.m_textures.empty();
	if (ImGui::Button("Upload Sprite(s)", btnSize)) {
		auto spritePaths = openFilePaths();
		if (spritePaths) {
			for (const auto& path : *spritePaths) {
				m_sprite.m_textures.emplace_back(
					std::make_shared<Texture>(IMG_LoadTexture(renderer.get(), path.c_str()))
				);
				m_texturePaths.push_back(path);
			}
		}
	}
	if (prevNoSprites && !m_sprite.m_textures.empty()) {
		renderer.addObj(&m_sprite, 0);
	}
	ImGui::End();
}

void SpriteEditor::selectSprite(Renderer& renderer) {
	static constexpr ImVec2 btnSize{ 270.0f, 50.0f };

	ImGui::SetNextWindowPos(spriteSwitchPos);
	ImGui::SetNextWindowSize(spriteSwitchSize);
	ImGui::Begin("Sprite Sheet");

	bool deleteSpriteInput = false;
	auto doomedSpriteIt = m_sprite.m_textures.begin();
	for (auto it = m_sprite.m_textures.begin(); it != m_sprite.m_textures.end(); it++) {
		ImGui::PushID(static_cast<int>(std::distance(m_sprite.m_textures.begin(), it)));
		if (ImGui::ImageButton(ImTextureID(m_xBtn.raw), xBtnSize)) {
			deleteSpriteInput = true;
			doomedSpriteIt = it;
		}
		ImGui::SameLine(70.0f); //show textures on the same line as the x-button

		if (ImGui::ImageButton(ImTextureID(it->get()->raw), spriteBtnSize) && !deleteSpriteInput) {
			m_sprite.changeTexture(std::distance(m_sprite.m_textures.begin(), it));
		}
		ImGui::PopID();
	}
	const bool prevHadSprites = !m_sprite.m_textures.empty();
	if (deleteSpriteInput) {
		m_sprite.m_textures.erase(doomedSpriteIt);
		if (m_sprite.m_currTexIdx > 0) {
			m_sprite.m_currTexIdx--;
		}
	}
	if (m_sprite.m_textures.empty() && prevHadSprites) {
		renderer.removeObj(m_sprite.getID(), 0);
	}
	
	ImGui::End();
}

nv::editor::SpriteEditor::SpriteEditor(Renderer& renderer) noexcept 
	: m_xBtn{ IMG_LoadTexture(renderer.get(), relativePath("/assets/x_btn.png").c_str()) }
{
}

nv::editor::EditorDest SpriteEditor::operator()(Renderer& renderer) {
	showSpriteOptions(renderer);
	selectSprite(renderer);
	return EditorDest::None;
}
