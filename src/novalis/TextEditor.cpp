#include "TextEditor.h"

nv::editor::TextEditor::TextEditor(ImGuiIO& io, SDL_Renderer* renderer)
	: Editor(io, renderer) {}

void nv::editor::TextEditor::showGui(bool& running)
{
	Text text;

	backgroundRectDrag.drag();

	ImGui::SetNextWindowPos(editPanelPos);
	ImGui::SetNextWindowSize(editPanelSize);
	ImGui::Begin("Text Properties");

	if (ImGui::InputText("Text", m_inputText.data(), maxMessageLength)) {
		text.setSize(m_fontSize, m_fontSize / 5);
		text.setText(m_renderer.get(), m_inputText.data());
	}
	if (ImGui::SliderInt("Font Size", &m_fontSize, 10, 1000)) {
		text.setSize(m_fontSize, m_fontSize / 5);
	} 
	if (ImGui::SliderInt("Wrap Length", &m_fontWrapLength, 10, 1000)) {
		text.setWrapLength(m_renderer.get(), static_cast<Uint32>(m_fontWrapLength));
	} 
	
	if (ImGui::ColorEdit3("Text Color", m_rgb.data())) {
		text.setColor(m_renderer.get(),
			SDL_Color{
				static_cast<Uint8>(m_rgb[0] * 255),
				static_cast<Uint8>(m_rgb[1] * 255),
				static_cast<Uint8>(m_rgb[2] * 255)
			}
		);
	}
	ImGui::End();

	ImGui::SetNextWindowPos(fontPanelPos);
	ImGui::SetNextWindowSize(fontPanelSize);
	ImGui::Begin("Font");

	if (ImGui::Button("Libertine", buttonSize)) {
		text.setFont(m_renderer.get(), FontType::Libertine);
	}
	if (ImGui::Button("Work Sans", buttonSize)) {
		text.setFont(m_renderer.get(), FontType::WorkSans);
	}

	ImGui::End();

	ImGui::SetNextWindowPos(rectPanelPos);
	ImGui::SetNextWindowSize(rectPanelSize);

	ImGui::Begin("Background Panel");
	editRect(text.backgroundRect());
	ImGui::End();

	ImGui::SetNextWindowPos(saveExitPanelPos);
	ImGui::SetNextWindowSize(saveExitPanelSize);

	ImGui::Begin("File");
	if (ImGui::Button("Load Text", buttonSize)) {
		auto filePath = openFilePath();
		if (filePath) {
			try {
				Text loaded{ m_renderer.get(), filePath.value() };
				text = std::move(loaded);
				text.setRenPos(600, 500);
				std::ranges::copy(text.text(), m_inputText.begin());
			} catch (std::runtime_error& e) {
				std::cerr << e.what() << std::endl;
			}
		}
	}
	if (ImGui::Button("Save", buttonSize)) {
		auto filePath = saveFile(L"Save File");
		if (filePath) {
			std::remove(filePath->c_str());
			std::ofstream file{ filePath.value() };
			file << text;
			file.close();
		}
	}
	ImGui::End();
}