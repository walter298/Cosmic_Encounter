#pragma once

#include "Scene.h"
#include "EditorUtil.h"

namespace nv {
	namespace editor {
		class SceneEditor {
		private:
			Scene::SpriteMap m_sprites;

			std::vector<Sprite*> m_overlappingSprites;
			size_t m_selectedSpriteIdx = 0;

			bool m_showingOptionToInsertSprite = false;
			ImVec2 m_currSpriteInsertionWindowPos{ 0.0f, 0.0f };

			void showSceneOptions();
			void checkForSpriteInsertion(Renderer& renderer);
			void editCurrSprite(Renderer& renderer);
		public:
			SceneEditor();
			EditorDest operator()(Renderer& renderer);
		};
	}
}

