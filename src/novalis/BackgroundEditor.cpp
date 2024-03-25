#include "BackgroundEditor.h"

using nv::editor::BackgroundEditor;

void BackgroundEditor::openBackground(EditorRenderer& renderer) {
	auto backgroundPath = openFilePath();
	if (backgroundPath) {
		try {
			Background newBackground{ renderer.get(), *backgroundPath };
			m_background = std::move(newBackground);
			m_background.m_x = BACKGROUND_X;
			m_background.m_y = BACKGROUND_Y;
		}
		catch (nlohmann::json::exception e) {
			std::cerr << e.what() << '\n';
		}
	}
}

void BackgroundEditor::saveBackground() {
	auto path = saveFile(L"Save Background");

	if (path) {
		nlohmann::json j;
		j["width"] = m_background.m_width;
		j["height"] = m_background.m_height;
		j["horizontal_texture_c"] = m_background.m_horizTexC;
		j["texture_paths"] = m_texturePaths;

		std::ofstream file{ *path };
		file << j.dump(2);
		file.close();
	} 
}

void BackgroundEditor::showOptions(EditorRenderer& renderer) {
	static constexpr ImVec2 dimWinPos{ 0.0f, 0.0f };
	static constexpr ImVec2 dimWinSize{ 300.0f, 400.0f };

	static constexpr ImVec2 btnSize{ 270.0f, 40.0f };

	ImGui::SetNextWindowPos(dimWinPos);
	ImGui::SetNextWindowSize(dimWinSize);

	ImGui::Begin("Options");
	if (ImGui::Button("Set Images", btnSize)) {
		loadImages(m_texturePaths, m_background.m_textures, renderer);
	}
	if (ImGui::Button("Clear Images", btnSize)) {
		m_texturePaths.clear();
		m_background.m_textures.clear();
	}
	if (ImGui::Button("Open Background", btnSize)) {
		openBackground(renderer);
	}
	if (ImGui::Button("Save Background", btnSize)) {
		saveBackground();
	}
	
	ImGui::InputInt("Width", &m_background.m_width);
	ImGui::InputInt("Height", &m_background.m_height);
	ImGui::InputInt("Horizontal Texture Count", &m_background.m_horizTexC);
	
	if (m_background.m_horizTexC <= 0) {
		m_background.m_horizTexC = 1;
	}

	ImGui::End();
}

BackgroundEditor::BackgroundEditor(EditorRenderer& renderer) {
	renderer.setBackground(&m_background);

	m_background.m_x = BACKGROUND_X;
	m_background.m_y = BACKGROUND_Y;

	m_background.m_horizTexC = 1;
	m_background.m_width = 200;
	m_background.m_height = 100;
}

nv::editor::EditorDest BackgroundEditor::operator()(EditorRenderer& renderer) {
	showOptions(renderer); 
	return EditorDest::None;
}