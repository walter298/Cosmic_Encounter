#ifndef SPRITE_EDITOR_H
#define SPRITE_EDITOR_H

#include <numeric>

#include <plf_hive.h>

#include "EditorUtil.h"
#include "Scene.h"

namespace nv {
	namespace editor {
		class SpriteEditor {
		private:
			static constexpr ImVec2 spriteBoxPos{
				centerPos(static_cast<float>(NV_SCREEN_WIDTH), 500.0f),
				centerPos(static_cast<float>(NV_SCREEN_HEIGHT), 500.0f)
			};
			static constexpr ImVec2 spriteBoxSize{ 500.0f, 500.0f };

			static constexpr ImVec2 spriteSwitchPos{
				spriteBoxPos.x + spriteBoxSize.x + 50.0f,
				spriteBoxPos.y
			};
			static constexpr ImVec2 spriteSwitchSize{ 400.0f, 400.0f };

			static constexpr ImVec2 xBtnSize{ 50.0f, 50.0f };
			static constexpr ImVec2 spriteBtnSize{ 300.0f, 50.0f };
			
			Texture m_xBtn;
			Sprite m_sprite;
			RectEditor m_spriteRectEditor;

			TexturePtrs& m_textures = m_sprite.m_textures;
			
			std::vector<std::string> m_texturePaths;

			void writeImagePathsAndSaveToFile(const std::string& savedPath, nlohmann::json& j);

			void showSpriteOptions(EditorRenderer& renderer);
			void selectSprite(EditorRenderer& renderer);
		public:
			SpriteEditor(EditorRenderer& renderer) noexcept;
			EditorDest operator()(EditorRenderer& renderer);
		};
	}
}

#endif