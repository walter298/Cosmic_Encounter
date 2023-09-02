#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include <array>

#include "EditorUtil.h"

namespace nv {
	namespace editor {
		class TextEditor : public Editor {
		private:
			static constexpr float editPanelX = 200.0f;
			static constexpr float editPanelY = 500.0f;
			static constexpr float editPanelW = 300.0f;
			static constexpr float editPanelH = 125.0f;

			static constexpr ImVec2 editPanelPos{ editPanelX, editPanelY };
			static constexpr ImVec2 editPanelSize{ editPanelW, editPanelH };

			static constexpr ImVec2 fontPanelPos{ editPanelX, editPanelY + editPanelH + 10.0f };
			static constexpr ImVec2 fontPanelSize{ 300.0f, 100.0f };

			static constexpr ImVec2 rectPanelSize{ 300.0f, 100.0f };
			static constexpr ImVec2 rectPanelPos{ editPanelX, fontPanelPos.y + fontPanelSize.y + 10.0f };

			static constexpr ImVec2 saveExitPanelPos{ rectPanelPos.x, rectPanelPos.y + rectPanelSize.y + 10.0f };
			static constexpr ImVec2 saveExitPanelSize{ editPanelW, 100.0f };

			static constexpr ImVec2 buttonSize{ 100.0f, 30.0f };

			int m_fontSize = 500;
			int m_fontWrapLength = 500;
			std::array<float, 3> m_rgb;

			static constexpr size_t maxMessageLength = 1024;
			std::array<char, maxMessageLength> m_inputText;

			Text m_text;
			DragCheck backgroundRectDrag = [this] {
				DragCheck drag;
				drag.add(m_text.backgroundRect());
				return drag;
			}();
		public:
			TextEditor(ImGuiIO& io, SDL_Renderer* renderer);
			void showGui(bool& running) override;
			void customRender() override {};
		};
	}
}

#endif