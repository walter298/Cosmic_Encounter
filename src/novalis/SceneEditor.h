#pragma once

#include "Scene.h"
#include "EditorUtil.h"

namespace nv {
	namespace editor {
		class SceneEditor {
		private:
			Layers<Sprite> m_sprites;
			Layers<Rect> m_rects;
			
			ObjectEditor m_objEditor;

			int m_currLayer = 0;

			bool m_showingRightClickOptions = false;
			ImVec2 m_rightClickWinPos{ 0.0f, 0.0f };

			template<RenderObject Obj>
			void insertObjFromFile(Renderer& renderer, plf::hive<Obj>& objects) {
				auto path = openFilePath();
				if (path) {
					try {
						auto spriteName = fileName(*path);
						objects.emplace(*path, renderer.get());
					} catch (std::exception e) {
						std::println("{}", e.what());
					}
				}
			}

			void showRightClickOptions(Renderer& renderer, const Coord& mousePos);

			void moveCamera(Renderer& renderer) noexcept;

			//void save();
		public:
			SceneEditor(Renderer& renderer);
			EditorDest operator()(Renderer& renderer);
		};
	}
}

