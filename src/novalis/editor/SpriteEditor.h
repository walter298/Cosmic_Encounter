#pragma once

#include "EditorUtil.h"
#include "ObjectSelection.h"

namespace nv {
	namespace editor {
		class SpriteEditor {
		private:
			SDL_Renderer* m_renderer;

			//ObjectLayers<EditedObjectData<Texture>> m_texLayers;
			//boost_con::flat_map<int, plf::hive<EditedObjectData<Texture>>> m_texLayers;
			ObjectSelection<Texture> m_objects;

			//SelectedObjectData<Texture> m_selectedTexObj;
			bool m_isTexSelected = false;

			int m_currLayer = 0;
			int m_currLayoutLayer = 0;

			void open(SDL_Renderer* renderer);
			void save();
			void saveAsTextureObject();
			void setIdenticalLayout();
			void showSpriteOptions(SDL_Renderer* renderer, SDL_Point mouse);
		public:
			SpriteEditor(SDL_Renderer* renderer) noexcept;
			EditorDest imguiRender();
			void sdlRender() const noexcept;
		};
	}
}


