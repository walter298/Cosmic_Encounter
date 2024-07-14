#pragma once

#include <unordered_map>

#include "EditorUtil.h"

namespace nv {
	namespace editor {
		class SpriteEditor {
		private:
			SDL_Renderer* m_renderer;

			Layers<EditedObjectData<TextureObject>> m_texLayers;
			SelectedObjectData<TextureObject> m_selectedTexObj;
			bool m_isTexSelected = false;

			int m_currLayer = 0;
			int m_currLayoutLayer = 0;

			void open(SDL_Renderer* renderer);
			void save();
			void saveAsTextureObject();
			void insertTextures(SDL_Renderer* renderer);
			void setIdenticalLayout();
			void showSpriteOptions(SDL_Renderer* renderer);
		public:
			SpriteEditor(SDL_Renderer* renderer) noexcept;
			EditorDest imguiRender();
			void sdlRender() const noexcept;
		};
	}
}


