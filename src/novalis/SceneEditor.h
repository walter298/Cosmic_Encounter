#pragma once

#include "Scene.h"
#include "EditorUtil.h"

namespace nv {
	namespace editor {
		class SceneEditor {
		private:
			Background m_background;
			
			struct SpriteData {
				Sprite sprite;
				int scale = 0;
				int layer = 0;
				int originalWidth = 0;
				int originalHeight = 0;
			};
			using Sprites = std::unordered_map<std::string, plf::hive<SpriteData>>;
			Sprites m_sprites;

			size_t m_selectedSpriteIdx = 0;

			bool m_showingRightClickOptions = false;
			ImVec2 m_rightClickWinPos{ 0.0f, 0.0f };

			//SpriteData
			SpriteData* m_selectedSpriteData = nullptr;
			Sprite* m_selectedSprite = nullptr;
			RectEditor m_spriteRectEditor;

			void resetSelectedSpriteData() noexcept;
			void saveSelectedSpriteData() noexcept;

			static constexpr ImVec2 backgroundOptionsWinPos{ 0.0f, static_cast<float>(NV_SCREEN_HEIGHT) - 300.0f };
			static constexpr ImVec2 backgroundOptionsWinSize{ 200.0f, 300.0f };

			void setBackground(EditorRenderer& renderer);
			void editBackground();
			void showBackgroundOptions(EditorRenderer& renderer);

			static constexpr ImVec2 spriteOptionsWinPos = adjacentPos(backgroundOptionsWinPos, backgroundOptionsWinSize);
			static constexpr ImVec2 spriteOptionsWinSize = backgroundOptionsWinSize;

			void insertSprite(EditorRenderer& renderer, const ImVec2& mousePos);
			void editSelectedSprite(EditorRenderer& renderer);
			
			static constexpr ImVec2 rectOptionsWinPos = adjacentPos(spriteOptionsWinPos, spriteOptionsWinSize);
			static constexpr ImVec2 rectOptionsWinSize = backgroundOptionsWinSize;

			plf::hive<Rect> m_rects;
			RectEditor m_rectEditor;

			void insertRect(EditorRenderer& renderer, const Coord& mousePos);
			void editSelectedRect(const Coord& mousePos);

			void selectObject(const Coord& mousePos);

			void showRightClickOptions(EditorRenderer& renderer, const Coord& mousePos);

			void moveCamera(EditorRenderer& renderer) noexcept;

			void save();
		public:
			SceneEditor(EditorRenderer& renderer);
			EditorDest operator()(EditorRenderer& renderer);
		};
	}
}

