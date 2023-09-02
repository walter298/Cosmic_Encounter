#ifndef SPRITE_EDITOR_H
#define SPRITE_EDITOR_H

#include <numeric>

#include "EditorUtil.h"
#include "Scene.h"

namespace nv {
	namespace editor {
		class SpriteEditor : public Editor {
		private:
			static constexpr ImVec2 spriteBoxPos{
				centerPos(static_cast<float>(NV_SCREEN_WIDTH), 500.0f),
				centerPos(static_cast<float>(NV_SCREEN_HEIGHT), 500.0f)
			};
			static constexpr ImVec2 spriteBoxSize{ 500.0f, 500.0f };

			static constexpr ImVec2 spriteOptionsPos{spriteBoxPos.x, spriteBoxPos.y + spriteBoxSize.y + 20.0f };
			static constexpr ImVec2 spriteOptionsSize{ spriteBoxSize.x, 400.0f };
			static constexpr ImVec2 spriteOptionsBtnSize{ 150.0f, 50.0f };
			static constexpr ImVec2 spriteSwitchPos{
				spriteBoxPos.x + spriteBoxSize.x + 50.0f,
				spriteBoxPos.y
			};
			static constexpr ImVec2 spriteSwitchSize{ 400.0f, 400.0f };

			static constexpr ImVec2 xBtnSize{ 50.0f, 50.0f };
			static constexpr ImVec2 spriteBtnSize{ 300.0f, 50.0f };

			static constexpr ImVec2 addOrRearrangeSpriteSize{ 250.0f, 147.0f };
			static constexpr ImVec2 addOrRearrangeSpritePos{
				spriteSwitchPos.x + centerPos(spriteSwitchSize.x, addOrRearrangeSpriteSize.x),
				spriteSwitchPos.y + spriteSwitchSize.y + 20.0f
			};

			std::unique_ptr<Texture> m_xBtn;
			std::vector<std::pair<std::string, std::unique_ptr<Texture>>> m_spriteTextures;
			size_t m_currSpriteTexIdx = 0;
			Rect m_rect{ 200, 200, 500, 500 };
		public:
			SpriteEditor(ImGuiIO& io, SDL_Renderer* renderer);
			void showGui(bool& running) override;
			void customRender() override;
		};
	}
}

#endif