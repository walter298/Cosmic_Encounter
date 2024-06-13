#include "SpriteEditor.h"

#include <print>

using nv::editor::SpriteEditor;

void nv::editor::SpriteEditor::insertTextures(Renderer& renderer) {
	auto images = openFilePaths();
	if (images) {
		TexturePos defaultPos;
		defaultPos.ren.setPos(400, 400);
		defaultPos.ren.setSize(300, 300);
		for (const auto& image : *images) {
			m_textures[m_currLayer].emplace(
				std::make_shared<Texture>(IMG_LoadTexture(renderer.get(), image.c_str())),
				defaultPos
			);
			renderer.add(&getBack(m_textures[m_currLayer]), m_currLayer);
			defaultPos.ren.rect.x += 300;
		}
	}
}

void nv::editor::SpriteEditor::setIdenticalLayout() {
	auto& texsToMove = m_textures[m_currLayer];
	auto& targetTexs = m_textures[m_currLayoutLayer];

	if (texsToMove.size() != targetTexs.size()) {
		std::println("Error: cannot model layout of layers with different # of textures");
		return;
	}
	for (auto [texToMove, targetTex] : views::zip(texsToMove, targetTexs)) {
		texToMove.setPos(targetTex.getPos());
	}
}

void SpriteEditor::showSpriteOptions(Renderer& renderer) {
	static constexpr ImVec2 layerOptionPos{ 0.0f, 0.0f };
	static constexpr ImVec2 layerOptionsSize{ 200.0f, 200.0f };

	ImGui::SetNextWindowPos(layerOptionPos);
	ImGui::SetNextWindowSize(layerOptionsSize);

	ImGui::Begin("Layer");

	//select layer
	if (ImGui::InputInt("Layer", &m_currLayer)) {
		m_texDataEditor.reseat(&m_textures[m_currLayer], m_currLayer);
		makeOneLayerMoreVisible(m_textures, m_currLayer, 100);
	}

	//insert textures
	if (ImGui::Button("Insert Texture(s)")) {
		insertTextures(renderer);
	}

	if (ImGui::Button("Set Layout")) {
		setIdenticalLayout();
	}
	ImGui::SameLine();
	ImGui::InputInt("Target Layer", &m_currLayoutLayer);
	
	if (ImGui::Button("Save")) {

	}
	ImGui::End();
}

SpriteEditor::SpriteEditor(Renderer& renderer) noexcept
	: m_texDataEditor{ renderer, { 0, 500 } }
{
	m_texDataEditor.reseat(&m_textures[m_currLayer], m_currLayer);
}

nv::editor::EditorDest SpriteEditor::operator()(Renderer& renderer) {
	showSpriteOptions(renderer);
	m_texDataEditor();
	return EditorDest::None;
}