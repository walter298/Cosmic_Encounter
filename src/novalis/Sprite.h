#pragma once

#include <fstream>
#include <string_view>
#include <optional>

#include <boost/unordered/unordered_flat_map.hpp>

#include <SDL2/SDL_image.h>

#include "data_util/BasicConcepts.h"
#include "Animation.h"
#include "Texture.h"

namespace nv {
	namespace editor { 
		class SceneEditor; 
		class SpriteEditor;
	}

	class Sprite : public detail::ObjectBase<Sprite> {
	private:
		boost_con::flat_map<int, std::vector<Texture>> m_texObjLayers;

		template<typename Animation>
		using Animations = boost::unordered_flat_map<ID<Animation>, Animation>;

		std::tuple<
			Animations<Animation>
		> m_animations;

		int m_currLayer = 0;
	public:
		Sprite(SDL_Renderer* renderer, const json& json, TextureMap& texMap);

		using JsonFormat = boost_con::flat_map<int, std::vector<std::pair<std::string, TextureData>>>;

		TextureData& getTexData(size_t texIdx);

		auto& getHitbox(this auto&& self, size_t index = 0) {
			return self.m_texObjLayers.at(m_currLayer)[index].getHitbox();
		}

		void setTextureLayer(int layer) noexcept;

		const std::vector<nv::Texture>& getTextures() const noexcept;

		void setPos(int destX, int destY) noexcept;
		void setPos(SDL_Point p) noexcept;

		void move(int x, int y) noexcept;
		void move(SDL_Point p) noexcept;

		void scale(int x, int y) noexcept;
		void scale(SDL_Point p) noexcept;

		void rotate(double angle, SDL_Point p);
		void setRotationCenter() noexcept;

		bool containsCoord(int x, int y) const noexcept;
		bool containsCoord(SDL_Point p) const noexcept;

		template<typename Animation>
		auto makeAnimation(Animation&& animation) {
			using PlainAnimation = std::remove_cvref_t<Animation>;

			ID<PlainAnimation> key;
			auto& animations = std::get<Animations<PlainAnimation>>(m_animations);

			animations.insert(std::pair{ key, std::forward<Animation>(animation) });
			
			return key;
		}

		template<typename Animation>
		size_t animate(ID<Animation> id) {
			auto& animations = std::get<Animations<Animation>>(m_animations);
			auto frame = animations.at(id)();
			setTextureLayer(frame);
			return frame;
		}

		template<typename Animation>
		void cancelAnimation(ID<Animation> id) {
			auto& animations = std::get<Animations<Animation>>(m_animations);
			animations.at(id).cancel();
		}

		void setOpacity(Uint8 opacity) noexcept;
		void flip(SDL_RendererFlip flip) noexcept;

		void render(SDL_Renderer* renderer) const noexcept;

		void save(json& json) const;
		static void saveJson(const Sprite& sprite, json& json);
		
		friend class editor::SceneEditor;
		friend class editor::SpriteEditor;
	};

	using SpriteRef = std::reference_wrapper<Sprite>;
}