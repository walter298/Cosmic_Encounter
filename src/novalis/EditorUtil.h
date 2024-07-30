#ifndef EDITOR_UTIL_H
#define EDITOR_UTIL_H

#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <print>
#include <string>
#include <thread> //sleep

#include <Windows.h>
#include <ShlObj.h>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <imgui_stdlib.h>

#include "Renderer.h"
#include "ID.h"
#include "Text.h"

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

		template<typename Func>
		struct ScopeExit {
		private:
			Func m_f;
		public:
			ScopeExit(const Func& f) noexcept(std::is_nothrow_copy_constructible_v<Func>) : m_f{ f } {}
			ScopeExit(Func&& f) noexcept(std::is_nothrow_move_constructible_v<Func>) : m_f{ std::move(f) } {}
			~ScopeExit() noexcept(std::is_nothrow_invocable_v<Func>) {
				m_f();
			}
		};

		template<typename T>
		concept Editor = requires(T t) { 
			{ t.imguiRender() } -> std::same_as<EditorDest>;
			t.sdlRender(); 
		};

		template<Editor RenderMethod>
		EditorDest runEditor(ImGuiIO& io, SDL_Renderer* renderer, RenderMethod& editor) {
			while (true) {
				constexpr auto waitTime = 1000ms / 180;
				const auto endTime = chrono::system_clock::now() + waitTime;

				SDL_Event evt;
				while (SDL_PollEvent(&evt)) {
					ImGui_ImplSDL2_ProcessEvent(&evt);
					if (evt.type == SDL_QUIT) {
						return EditorDest::Quit;
					} else if (evt.type == SDL_KEYDOWN) {
						if (evt.key.keysym.scancode == SDL_SCANCODE_MINUS) {
							return EditorDest::Home;
						}
					}
				}

				SDL_RenderClear(renderer);
				editor.sdlRender();
				
				static constexpr ImVec4 color{ 0.45f, 0.55f, 0.60f, 1.00f };
				
				ImGui_ImplSDLRenderer2_NewFrame();
				ImGui_ImplSDL2_NewFrame();
				ImGui::NewFrame();

				auto dest = editor.imguiRender();

				ImGui::Render();
				SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
				SDL_SetRenderDrawColor(renderer,
					//unfortunately SDL uses ints for screen pixels and ImGui uses floats 
					static_cast<Uint8>(color.x * 255), static_cast<Uint8>(color.y * 255),
					static_cast<Uint8>(color.z * 255), static_cast<Uint8>(color.w * 255));
				ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
				SDL_RenderPresent(renderer);
				if (dest != EditorDest::None) {
					return dest;
				}
			}
		}

		void runEditors();

		std::optional<std::string> openFilePath();
		std::optional<std::vector<std::string>> openFilePaths();
		std::optional<std::string> saveFile(std::wstring openMessage);

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

		template<typename EditedObject>
		auto selectObj(std::vector<EditedObject>& objs, SDL_Point mousePos) {
			return ranges::find_if(objs, [&](const auto& editedObj) {
				return editedObj.obj.containsCoord(mousePos);
			});
		}

		template<RenderObject Object>
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

		template<RenderObject Object>
		struct SelectedObjectData {
			EditedObjectData<Object>* obj                   = nullptr;
			std::vector<EditedObjectData<Object>>* objLayer = nullptr;
			std::vector<EditedObjectData<Object>>::iterator it;

			void resetToLastElement(std::vector<EditedObjectData<Object>>* newObjLayer) {
				obj      = &newObjLayer->back();
				objLayer = newObjLayer;
				it       = std::prev(std::end(*newObjLayer));
			}
		};

		template<RenderObject Object>
		void edit(SelectedObjectData<Object>& editedObj) {
			auto mousePos = convertPair<SDL_Point>(ImGui::GetMousePos());
			if (editedObj.obj->obj.containsCoord(mousePos)) {
				auto mouseChange = convertPair<SDL_Point>(ImGui::GetMouseDragDelta());
				editedObj.obj->obj.move(mouseChange);
				ImGui::ResetMouseDragDelta();
			}

			//if we are editing text
			if constexpr (std::same_as<Object, Text>) {
				std::string temp = editedObj.obj->obj.value().data();
				if (ImGui::InputText("Value", &temp)) {
					editedObj.obj->obj = temp;
				}
			}

			ImGui::Text("Size");

			//setting size
			if constexpr (SizeableObject<Object>) {
				if (ImGui::InputInt("width", &editedObj.obj->width)) {
					editedObj.obj->obj.setSize(editedObj.obj->width, editedObj.obj->height);
				}
				if (ImGui::InputInt("height", &editedObj.obj->height)) {
					editedObj.obj->obj.setSize(editedObj.obj->width, editedObj.obj->height);
				}
			}

			//scaling texture
			int oldScale = editedObj.obj->scale;
			if (ImGui::SliderInt("Scale", &editedObj.obj->scale, 0, 1500)) {
				int deltaScale = editedObj.obj->scale - oldScale;
				editedObj.obj->obj.scale(deltaScale, deltaScale);
			}

			//rotation
			if constexpr (RotatableObject<Object>) {
				ImGui::Text("Rotation");
				auto floatAngle = static_cast<float>(editedObj.obj->angle);
				if (ImGui::SliderFloat("Angle", &floatAngle, 0.0f, 360.0f)) {
					editedObj.obj->obj.rotate(static_cast<double>(floatAngle), editedObj.obj->rotationPoint);
				}
				if (ImGui::InputInt("Rotation x", &editedObj.obj->rotationPoint.x)) {
					editedObj.obj->obj.rotate(static_cast<double>(floatAngle), editedObj.obj->rotationPoint);
				}
				if (ImGui::InputInt("Rotation y", &editedObj.obj->rotationPoint.y)) {
					editedObj.obj->obj.rotate(static_cast<double>(floatAngle), editedObj.obj->rotationPoint);
				}
				editedObj.obj->angle = static_cast<double>(floatAngle);
			}

			if (ImGui::InputText("Name", &editedObj.obj->name)) {
				return;
			}

			//duplication 
			if constexpr (std::copyable<Object>) {
				if (ImGui::Button("Duplicate")) {
					editedObj.objLayer->push_back(*editedObj.obj);
					editedObj.obj = &editedObj.objLayer->back();
					editedObj.it = std::prev(editedObj.objLayer->end());
				}
			}

			//deletion
			if (ImGui::Button("Delete")) {
				editedObj.objLayer->erase(editedObj.it);
				editedObj.obj = nullptr;
			}
		}

		template<RenderObject Object>
		void makeOneLayerMoreVisible(Layers<EditedObjectData<Object>>& objLayers, int visibleLayer, Uint8 reducedOpacity) {
			auto reduceOpacity = [&](auto range) {
				for (auto& [layer, objLayer] : range) {
					for (auto& editedObj : objLayer) {
						editedObj.obj.setOpacity(reducedOpacity);
					}
				}
			};
			
			//set to full opacity in case it was already reduced before we called this function
			for (auto& editedObj : objLayers[visibleLayer]) {
				editedObj.obj.setOpacity(255);
			}

			auto visibleLayerIt = objLayers.find(visibleLayer);
			auto beforeVisibleLayer = ranges::subrange(objLayers.begin(), visibleLayerIt);
			auto afterVisibleLayer = ranges::subrange(std::next(visibleLayerIt), objLayers.end());
			reduceOpacity(beforeVisibleLayer);
			reduceOpacity(afterVisibleLayer);
		}
		
		template<RenderObject... Objects>
		void renderCopy(const Layers<EditedObjectData<Objects>>&... objLayers) {
			auto renderImpl = [&](const auto& layers) {
				for (const auto& [layer, objLayer] : layers) {
					for (const auto& editedObj : objLayer) {
						editedObj.obj.render();
					}
				}
			};
			((renderImpl(objLayers)), ...);
		}

		template<RenderObject Object>
		void saveObjects(const Layers<EditedObjectData<Object>>& objLayers, json& objLayersRoot) {
			for (const auto& [layer, objs] : objLayers) {
				auto& newLayerJson = objLayersRoot.emplace_back();
				newLayerJson["layer"] = layer;
				
				auto& objsJson = newLayerJson["objects"];
				objsJson = json::array();
				for (const auto& editedObj : objs) {
					auto& objJson = objsJson.emplace_back();
					editedObj.obj.save(objJson);
					objJson["name"] = editedObj.name;
				}
			}
		}

		template<RenderObject... Objects>
		void cameraMove(int dx, int dy, Layers<EditedObjectData<Objects>>&... objLayers) {
			auto moveImpl = [&](auto& layers) {
				for (auto& [layer, objLayer] : layers) {
					for (auto& objData : objLayer) {
						objData.obj.move(dx, dy);
					}
				}
			};
			((moveImpl(objLayers)), ...);
		}
	}
}

#endif