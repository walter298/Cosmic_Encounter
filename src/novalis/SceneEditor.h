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
			
			ObjectLayers<EditedObjectData<Sprite>, EditedObjectData<Texture>, 
				EditedObjectData<Text>, EditedObjectData<Rect>> m_objectLayers;
			
			enum class SelectedObjectType {
				Sprite,
				Texture,
				Text,
				Rect,
				None
			};
			SelectedObjectType m_selectedObjType = SelectedObjectType::None;
			TypeMap<SelectedObjectType, Sprite, Texture, Text, Rect> m_selectedObjMap;

			SelectedObjectData<Sprite> m_selectedSpriteData;
			SelectedObjectData<Texture> m_selectedTextureData;
			SelectedObjectData<Text> m_selectedTextData;
			SelectedObjectData<Rect> m_selectedRectData;

			bool m_showingFontOptions = false;
			int m_fontSize = 10;
			std::string m_fontPath;

			int m_currLayer = 0;
			
			bool m_showingRightClickOptions = false;
			ImVec2 m_rightClickWinPos{ 0.0f, 0.0f };

			void reduceOpacityOfOtherLayers();
			void createRect();
			void loadSprite();
			void showFontOptions();
			void createTextures() noexcept;
			void showRightClickOptions() noexcept;
			void save() const noexcept;
			void showSceneOptions() noexcept;

			template<typename Object>
			void selectImpl(EditedObjectVector<Object>& objLayer, SelectedObjectData<Object>& selectedObjData) {
				if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
					auto selectedObj = selectObj(objLayer, convertPair<SDL_Point>(ImGui::GetMousePos()));
					if (selectedObj != objLayer.end()) {
						selectedObjData.obj = &(*selectedObj);
						selectedObjData.objLayer = &objLayer;
						selectedObjData.it = selectedObj;
						m_selectedObjType = m_selectedObjMap.get<Object>();
					}
				}
			}

			void editLayers();
		public:
			SceneEditor(SDL_Renderer* renderer);
			EditorDest imguiRender();
			void sdlRender() const noexcept;
		};
	}
}