#ifndef EDITOR_UTIL_H
#define EDITOR_UTIL_H

#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <print>
#include <string>

#include <imgui.h>
#include <imgui_stdlib.h>

#include "../data_util/BasicConcepts.h"
#include "../Text.h"

namespace nv {
	namespace editor {
		enum class EditorDest {
			None,
			Quit,
			Scene,
			Sprite,
			Text,
			Home
		};

		template<typename T>
		constexpr auto centerPos(T l1, T l2) {
			return (l1 - l2) / 2;
		}

		constexpr ImVec2 adjacentPos(const ImVec2& pos, const ImVec2& size, float spacing = 0.0f) {
			return ImVec2{ pos.x + size.x + spacing,  pos.y };
		}
		constexpr ImVec2 buttonList(const ImVec2& btnSize, int btnC) noexcept {
			return {
				btnSize.x + 15,
				btnSize.y * static_cast<float>(btnC) + 40.0f
			};
		}

		//used to convert std::pair<int, int> <----> ImVec2
		template<typename Ret, typename Pair>
		Ret convertPair(const Pair& pair) noexcept {
			using Converted = std::remove_cvref_t<decltype(Ret::x)>;
			return Ret{ static_cast<Converted>(pair.x), static_cast<Converted>(pair.y) };
		}

		/*template<typename Object>
		auto selectObj(EditedObjectHive<Object>& objs, SDL_Point mousePos) {
			return ranges::find_if(objs, [&](const auto& editedObj) {
				return editedObj.obj.containsCoord(mousePos);
			});
		}*/

		template<typename Object>
		struct EditedObjectData {
			Object obj;
			int scale = 0;
			int width = 0;
			int height = 0;
			double angle = 0;
			SDL_Point rotationPoint{ 0, 0 };
			std::string name{ "name" };

			template<typename... Args>
			constexpr EditedObjectData(Args&&... args) requires(std::constructible_from<Object, Args...>)
				: obj{ std::forward<Args>(args)... }
			{
				if constexpr (SizeableObject<Object>) {
					auto size = obj.getSize();
					width = size.x;
					height = size.y;
				}
			}
		};

		template<typename Object>
		using EditedObjectHive = plf::hive<EditedObjectData<Object>>;
	}
}

#endif