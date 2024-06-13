#pragma once

#include "EditorUtil.h"

namespace nv {
	namespace editor {
		class SpriteEditor {
		private:
			Layers<TextureData> m_textures;
			int m_currLayer = 0;
			int m_currLayoutLayer = 0;

			ObjectEditor m_texDataEditor;
			
			void insertTextures(Renderer& renderer);
			void setIdenticalLayout();
			void showSpriteOptions(Renderer& renderer);
		public:
			SpriteEditor(Renderer& renderer) noexcept;
			~SpriteEditor() { std::println("Destroying Sprite Editor\n"); }
			EditorDest operator()(Renderer& renderer);
		};
	}
}


