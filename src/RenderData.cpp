#include "RenderData.h"

#include "novalis/data_util/File.h"
#include "novalis/data_util/BasicJsonSerialization.h"

namespace {
	CardMap loadCardMap(SDL_Renderer* renderer) {
		CardMap ret;

		namespace fs = std::filesystem;

		nv::TextureData texData;
		texData.ren.setSize(200, 200);

		//load negotiate
		auto negPath = nv::relativePath("Cosmic_Encounter/game_assets/images/cards/negotiate/negotiate.png");
		nv::SharedTexture tex{ IMG_LoadTexture(renderer, negPath.c_str()), SDL_DestroyTexture };
		ret.emplace(std::piecewise_construct,
			std::forward_as_tuple(Card::Negotiate),
			std::forward_as_tuple(renderer, negPath, tex, texData)
		);

		auto loadImpl = [&](const fs::directory_entry& image, Card::Type type) {
			auto pathStr = image.path().string();
			int attack = std::stoi(image.path().filename().string());
			nv::SharedTexture tex{ IMG_LoadTexture(renderer, pathStr.c_str()), SDL_DestroyTexture };
			ret.emplace(std::piecewise_construct,
				std::forward_as_tuple(type, attack),
				std::forward_as_tuple(renderer, pathStr, tex, texData)
			);
		};

		//load sprites and reinforcements
		for (const auto& image : fs::directory_iterator{ nv::relativePath("Cosmic_Encounter/game_assets/images/cards/attack") }) {
			loadImpl(image, Card::Attack);
		}
		for (const auto& image : fs::directory_iterator{ nv::relativePath("Cosmic_Encounter/game_assets/images/cards/reinforcement") }) {
			loadImpl(image, Card::Reinforcement);
		}

		return ret;
	}
}

GameRenderData::GameRenderData(SDL_Renderer* renderer) noexcept
	: renderer{ renderer }
{
}
