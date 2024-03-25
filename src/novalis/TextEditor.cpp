#include "TextEditor.h"

using nv::editor::TextEditor;

TextEditor::TextEditor(ImGuiIO& io, SDL_Renderer* renderer)
	//: Editor(io, renderer) 
{
	/*m_text.m_font = Text::fontMap.at(FontType::Libertine);
	m_text.setRenPos(static_cast<int>(editPanelX + editPanelW + 20.0f), static_cast<int>(editPanelY));
	m_renderer.addObj(&m_text, 1);*/
}

void nv::editor::TextEditor::showGui(bool& running)
{
	//backgroundRectDrag.drag();

	//ImGui::SetNextWindowPos(editPanelPos);
	//ImGui::SetNextWindowSize(editPanelSize);
	//ImGui::Begin("Text Properties");

	//if (ImGui::InputText("Text", m_textBuf.data(), maxMessageLength)) {
	//	//m_text.changeText(m_renderer.get(), m_textBuf.data());
	//}

	//if (ImGui::SliderInt("Font Size", &m_text.m_fontSize, 10, 1000)) {
	//	//m_text.setFontSize(m_renderer.get(), m_text.m_fontSize);
	//} 

	/*std::array rgb = {
		static_cast<float>(m_text.m_color.r / 255),
		static_cast<float>(m_text.m_color.g / 255),
		static_cast<float>(m_text.m_color.b / 255)
	};
	if (ImGui::ColorEdit3("Text Color", rgb.data())) {
		m_text.setColor({
			static_cast<Uint8>(rgb[0] * 255),
			static_cast<Uint8>(rgb[1] * 255),
			static_cast<Uint8>(rgb[2] * 255) }
		);
	}*/
	ImGui::End();

	ImGui::SetNextWindowPos(fontPanelPos);
	ImGui::SetNextWindowSize(fontPanelSize);
	ImGui::Begin("Font");

	if (ImGui::Button("Libertine", buttonSize)) {
		//m_text.m_font = Text::fontMap.at(FontType::Libertine);
	}
	if (ImGui::Button("Work Sans", buttonSize)) {
		//m_text.m_font = Text::fontMap.at(FontType::WorkSans);
	}

	ImGui::End();

	ImGui::SetNextWindowPos(rectPanelPos);
	ImGui::SetNextWindowSize(rectPanelSize);

	ImGui::Begin("Background Panel");
	//editRect(m_text.m_background);
	ImGui::End();

	ImGui::SetNextWindowPos(saveExitPanelPos);
	ImGui::SetNextWindowSize(saveExitPanelSize);

	ImGui::Begin("File");
	if (ImGui::Button("Load Text", buttonSize)) {
		auto filePath = openFilePath();
		if (filePath) {
			try {
				//Text loaded{ m_renderer.get(), *filePath };
				//m_text = std::move(loaded);
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
			//file << m_text;
			file.close();
		}
	}
	ImGui::End();
}