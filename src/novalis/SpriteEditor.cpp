#include "SpriteEditor.h"

using nv::editor::SpriteEditor;

SpriteEditor::SpriteEditor(ImGuiIO& io, SDL_Renderer* renderer)
	: Editor(io, renderer)
{
	m_xBtn = std::make_unique<Texture>(IMG_LoadTexture(
		renderer, (workingDirectory() + "novalis_assets/sprite_editor/x_button.png").c_str())
	);
}

void SpriteEditor::showGui(bool& running) {
	ImGui::SetNextWindowPos(spriteOptionsPos);
	ImGui::SetNextWindowSize(spriteOptionsSize);
	ImGui::Begin("Sprite Options");
	editRect(m_rect);
	if (ImGui::Button("Save Sprite", spriteOptionsBtnSize)) {
		auto savedPath = saveFile(L"Save Sprite");
		if (savedPath) {
			json j;
			j["image_paths"] = std::vector<std::string>();
			j.at("image_paths").get<std::vector<std::string>>().reserve(m_spriteTextures.size());
			for (const auto& [path, tex] : m_spriteTextures) {
				j.at("image_paths").push_back(path);
			}
			j["ren"] = m_rect;
			std::ofstream spriteFile{ savedPath.value() };
			assert(spriteFile.is_open());
			spriteFile << j.dump(3);
			spriteFile.close();
		}
	}
	ImGui::End();
	
	ImGui::SetNextWindowPos(spriteSwitchPos);
	ImGui::SetNextWindowSize(spriteSwitchSize);
	ImGui::Begin("Select Sprite");

	bool deleteSpriteInput = false;
	auto doomedSpriteIt = m_spriteTextures.begin();
	for (auto it = m_spriteTextures.begin(); it != m_spriteTextures.end(); it++) {
		ImGui::PushID(static_cast<int>(std::distance(m_spriteTextures.begin(), it)));
		if (ImGui::ImageButton(ImTextureID(m_xBtn->raw), xBtnSize)) {
			deleteSpriteInput = true;
			doomedSpriteIt = it;
		}
		ImGui::SameLine(70.0f);
		
		if (ImGui::ImageButton(ImTextureID(it->second->raw), spriteBtnSize) && !deleteSpriteInput) {
			m_currSpriteTexIdx = std::distance(m_spriteTextures.begin(), it);
		}
		ImGui::PopID();
	}
	if (deleteSpriteInput) {
		m_spriteTextures.erase(doomedSpriteIt);
		if (m_currSpriteTexIdx > 0) {
			m_currSpriteTexIdx--;
		}
	}

	ImGui::End();

	//constexpr ImVec2 addOrRearrangeBtnSize{}
	ImGui::SetNextWindowPos(addOrRearrangeSpritePos);
	ImGui::SetNextWindowSize(addOrRearrangeSpriteSize);
	ImGui::Begin("Add/Rearrange Sprites");

	constexpr ImVec2 addOrRearrangeSpriteBtnSize{ addOrRearrangeSpriteSize.x - 18.0f, 50.0f };
	if (ImGui::Button("Upload Sprite(s)", addOrRearrangeSpriteBtnSize)) {
		auto spritePaths = openFilePaths();
		if (spritePaths) {
			for (const auto& path : *spritePaths) {
				m_spriteTextures.push_back(
					std::pair{ 
						path, 
						std::make_unique<Texture>(IMG_LoadTexture(m_renderer.get(), path.c_str()))
					}
				);
			}
		}
	}
	if (ImGui::Button("Rearrange Sprites", addOrRearrangeSpriteBtnSize)) {

	}
	ImGui::End();
}

void SpriteEditor::customRender() {
	if (m_spriteTextures.size() > 0) {
		SDL_RenderCopy(m_renderer.get(), m_spriteTextures[m_currSpriteTexIdx].second->raw, nullptr, &m_rect.rect);
	}
}

