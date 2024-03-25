#pragma once

#include "EditorUtil.h"

namespace nv {
	namespace editor {
		class BackgroundEditor {
		private:
			static constexpr int BACKGROUND_X = 400;
			static constexpr int BACKGROUND_Y = 0;

			Background m_background;
			std::vector<std::string> m_texturePaths;
			void openBackground(EditorRenderer& renderer);
			void saveBackground();
			void showOptions(EditorRenderer& renderer);
		public:
			BackgroundEditor(EditorRenderer& renderer);
			EditorDest operator()(EditorRenderer& renderer);
		};
	}
}