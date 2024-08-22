#pragma once

#include "EditorUtil.h"

namespace nv {
	namespace editor {
		class SpriteEditor {
		private:
			SDL_Renderer* m_renderer;

			boost_con::flat_map<int, plf::hive<EditedObjectData<Texture>>> m_texLayers;
			SelectedObjectData<Texture> m_selectedTexObj;
			bool m_isTexSelected = false;

			int m_currLayer = 0;
			int m_currLayoutLayer = 0;

			void reduceOpacityOfOtherLayers();
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


