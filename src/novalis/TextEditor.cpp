#include "TextEditor.h"

void nv::editor::runTextEditor(Text& text, nv::Renderer& renderer, bool& running)
{
	constexpr float editPanelX = 200.0f;
	constexpr float editPanelY = 500.0f;
	constexpr float editPanelW = 300.0f;
	constexpr float editPanelH = 125.0f;

	constexpr ImVec2 editPanelPos{ editPanelX, editPanelY };
	constexpr ImVec2 editPanelSize{ editPanelW, editPanelH };
	
	constexpr ImVec2 fontPanelPos{ editPanelX, editPanelY + editPanelH + 10.0f };
	constexpr ImVec2 fontPanelSize{ 300.0f, 100.0f };

	constexpr ImVec2 rectPanelSize{ 300.0f, 100.0f };
	constexpr ImVec2 rectPanelPos{ editPanelX, fontPanelPos.y + fontPanelSize.y + 10.0f };

	constexpr ImVec2 saveExitPanelPos{ rectPanelPos.x, rectPanelPos.y + rectPanelSize.y + 10.0f };
	constexpr ImVec2 saveExitPanelSize{ editPanelW, 100.0f };

	constexpr ImVec2 buttonSize{ 100.0f, 30.0f };

	static int fontSize = 500;
	static int fontWrapLength = 500;
	static std::array<float, 3> rgb;

	constexpr size_t maxMessageLength = 1024;
	static std::array<char, maxMessageLength> inputText;

	static DragCheck backgroundRectDrag = [&text] {
		DragCheck drag;
		drag.add(text.backgroundRect());
		return drag;
	}();

	backgroundRectDrag.drag();

	ImGui::SetNextWindowPos(editPanelPos);
	ImGui::SetNextWindowSize(editPanelSize);
	ImGui::Begin("Text Properties");

	if (ImGui::InputText("Text", inputText.data(), maxMessageLength)) {
		text.setSize(fontSize, fontSize / 5);
		text.setText(renderer.get(), inputText.data());
	}
	if (ImGui::SliderInt("Font Size", &fontSize, 10, 1000)) {
		text.setSize(fontSize, fontSize / 5);
	} 
	if (ImGui::SliderInt("Wrap Length", &fontWrapLength, 10, 1000)) {
		text.setWrapLength(renderer.get(), static_cast<Uint32>(fontWrapLength));
	} 
	
	if (ImGui::ColorEdit3("Text Color", rgb.data())) {
		text.setColor(renderer.get(),
			SDL_Color{
				static_cast<Uint8>(rgb[0] * 255),
				static_cast<Uint8>(rgb[1] * 255),
				static_cast<Uint8>(rgb[2] * 255)
			}
		);
	}
	ImGui::End();

	ImGui::SetNextWindowPos(fontPanelPos);
	ImGui::SetNextWindowSize(fontPanelSize);
	ImGui::Begin("Font");

	if (ImGui::Button("Libertine", buttonSize)) {
		text.setFont(renderer.get(), FontType::Libertine);
	}
	if (ImGui::Button("Work Sans", buttonSize)) {
		text.setFont(renderer.get(), FontType::WorkSans);
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
				Text loaded{ renderer.get(), filePath.value() };
				text = std::move(loaded);
				text.setRenPos(600, 500);
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