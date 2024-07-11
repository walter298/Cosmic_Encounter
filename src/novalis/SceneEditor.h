#pragma once

#include "Scene.h"
#include "EditorUtil.h"

namespace nv {
	namespace editor {
		class SceneEditor {
		private:
			SDL_Renderer* m_renderer;
			TextureMap m_texMap;
			FontMap m_fontMap;
			
			Layers<EditedObjectData<Sprite>> m_spriteLayers;
			Layers<EditedObjectData<TextureObject>> m_texObjLayers;
			Layers<EditedObjectData<Text>> m_textLayers;
			Layers<EditedObjectData<Rect>> m_rectLayers;

			enum class SelectedObjectType {
				Sprite,
				Texture,
				Text,
				Rect,
				None
			};
			SelectedObjectType m_selectedObjType = SelectedObjectType::None;
			SelectedObjectData<Sprite> m_selectedSpriteData;
			SelectedObjectData<TextureObject> m_selectedTexObjData;
			SelectedObjectData<Text> m_selectedTextData;
			SelectedObjectData<Rect> m_selectedRectData;

			bool m_showingFontOptions = false;
			int m_fontSize = 10;
			std::string m_fontPath;

			int m_currLayer = 0;
			
			bool m_showingRightClickOptions = false;
			ImVec2 m_rightClickWinPos{ 0.0f, 0.0f };

			void createRect();
			void loadSprite();
			void showFontOptions();
			void createTextures() noexcept;
			void showRightClickOptions() noexcept;
			void save() const noexcept;
			void showSceneOptions() noexcept;
			void editLayers();
		public:
			SceneEditor(SDL_Renderer* renderer);
			EditorDest imguiRender();
			void sdlRender() const noexcept;
		};
	}
}