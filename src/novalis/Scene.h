#ifndef SCENE_H
#define SCENE_H

#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <ranges>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

#include "Event.h"
#include "Instance.h"
#include "Button.h"
#include "GlobalMacros.h"
#include "Sprite.h"

namespace nv {
	namespace editor {
		class SceneEditor;
	}

	class Scene {
	public:
		enum class EndReason {
			QuitGame,
			NextScene
		};
	private:
		EndReason m_endReason = EndReason::QuitGame;
		bool m_running = false;

		using SpriteMap = std::unordered_map<std::string, Sprites>;
		SpriteMap m_sprites;
	public:
		EventHandler eventHandler;
		Renderer renderer;

		Scene() = default;
		Scene(Instance& instance);
		Scene(const std::string& path, Instance& instance);

		Sprite& sprite(const std::string& name);
		Sprites& spriteClones(const std::string& name);

		EndReason endReason() const noexcept;
		void endScene(EndReason end) noexcept;

		void execute();

		friend class editor::SceneEditor;
	};
}

#endif