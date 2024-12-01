#pragma once

#include <ranges>
#include <variant>

#include <imgui.h>

#include "../data_util/Algorithms.h"
#include "../data_util/DataStructures.h"
#include "../data_util/File.h"
#include "PolygonBuilder.h"
#include "EditorUtil.h"

namespace nv {
	namespace editor {
		template<typename... Ts>
		class ObjectSelection : public ObjectLayers<EditedObjectData<Polygon>, EditedObjectData<Ts>...> {
		private:
			template<typename Object>
			struct SelectedObjectData {
				EditedObjectData<Object>* obj = nullptr;
				EditedObjectHive<Object>* objLayer = nullptr;
				EditedObjectHive<Object>::iterator it;

				void resetToRandomElement(EditedObjectHive<Object>* newObjLayer) {
					obj = &(*newObjLayer->begin());
					objLayer = newObjLayer;
					it = newObjLayer->begin();
				}
				void reset() {
					obj      = nullptr;
					objLayer = nullptr;
				}

				void select(EditedObjectHive<Object>& hive, EditedObjectHive<Object>::iterator it) {
					obj = &(*it);
					objLayer = &hive;
					it = it;
				}
			};

			bool m_draggingObject = true;

			using SelectedObjectVariant = std::variant<std::monostate, SelectedObjectData<Polygon>, SelectedObjectData<Ts>...>;
			SelectedObjectVariant m_selectedObject = std::monostate{};

			PolygonBuilder m_polygonBuilder;

			template<typename Object>
			void moveObjectByMouseDragDelta(EditedObjectData<Object>& editedObj) {
				auto mousePos = convertPair<SDL_Point>(ImGui::GetMousePos());
				if (editedObj.obj->obj.containsCoord(mousePos)) {
					auto mouseChange = convertPair<SDL_Point>(ImGui::GetMouseDragDelta());
					editedObj.obj->obj.move(mouseChange);
					ImGui::ResetMouseDragDelta();
				}
			}

			template<typename Object>
			void showObjectRotationOption(SelectedObjectData<Object>& editedObj) {
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

			template<typename Object>
			void showObjectDuplicationOption(SelectedObjectData<Object>& editedObj) {
				if (ImGui::Button("Duplicate")) {
					auto it = editedObj.objLayer->insert(*editedObj.obj);
					editedObj.obj = &(*it);
					editedObj.it = it;
				}
			}

			template<typename Object>
			void showObjectDeletionOption(SelectedObjectData<Object>& editedObj) {
				if (ImGui::Button("Delete")) {
					editedObj.objLayer->erase(editedObj.it);
					editedObj.obj = nullptr;
					deselectSelectedObject();
				}
			}

			template<RenderObject Object>
			void edit(SDL_Point mousePos, SelectedObjectData<Object>& editedObj) {
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
				if constexpr (ScaleableObject<Object>) {
					int oldScale = editedObj.obj->scale;
					if (ImGui::SliderInt("Scale", &editedObj.obj->scale, 0, 1500)) {
						int deltaScale = editedObj.obj->scale - oldScale;
						editedObj.obj->obj.scale(deltaScale, deltaScale);
					}
				}

				//rotation
				if constexpr (RotatableObject<Object>) {
					showObjectRotationOption(editedObj);
				}

				if (ImGui::InputText("Name", &editedObj.obj->name)) {
					return;
				}

				//duplication 
				if constexpr (std::copyable<Object>) {
					showObjectDuplicationOption(editedObj);
				}

				//deletion
				showObjectDeletionOption(editedObj);
			}
		public:
			ObjectSelection(SDL_Renderer* renderer) 
				: m_polygonBuilder{ renderer } 
			{
			};

			template<std::same_as<Texture> T>
			void addFromFiles(int layer, SDL_Renderer* renderer, SDL_Point mouse) {
				auto texPaths = openMultipleFiles({ { "Images", "png" } });
				if (!texPaths) {
					return;
				}

				TextureData currTexData;
				currTexData.ren.setPos(mouse);

				auto& currLayer = std::get<EditedObjectHive<Texture>>(this->layers[layer]);
				
				auto setTexSize = [&currTexData](auto& texPtr) {
					int w = 0;
					int h = 0;
					SDL_QueryTexture(texPtr.get(), nullptr, nullptr, &w, &h);
					currTexData.ren.setSize(w, h);
					currTexData.world.setSize(w, h);
				};

				for (const auto& texPath : *texPaths) {
					auto sdlTexPtr = loadSharedTexture(renderer, texPath);
					setTexSize(sdlTexPtr);
					auto& tex = *(currLayer.emplace(
						renderer,
						texPath, //todo: make texPath part of TextureObject constructor
						std::move(sdlTexPtr),
						currTexData
					));
					tex.name = fileName(texPath);
				}
				auto begin = std::begin(currLayer);
				m_selectedObject = SelectedObjectData{
					&(*begin), &currLayer, begin
				};
			}

			template<std::ranges::viewable_range Objects>
			bool selectObjectImpl(SDL_Point mouse, Objects& objs) {
				//find first object that is clicked on
				auto selectedObjIt = std::ranges::find_if(objs, [&](const auto& editedObjData) -> bool {
					return editedObjData.obj.containsCoord(mouse);
				});
				if (selectedObjIt == objs.end()) {
					return false;
				}
				m_selectedObject = SelectedObjectData{
					&(*selectedObjIt), &objs, selectedObjIt
				};
				
				return true;
			}

			void selectObject(int layer, SDL_Point mouse) {
				if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
					m_draggingObject = false;
					return;
				/*if we have not released the button and we are dragging an object,
				then we don't need to select a new object*/
				} else if (!ImGui::IsMouseDown(ImGuiMouseButton_Left) || m_draggingObject) {
					return;
				}

				this->forEachHive([&, this](int layer, auto& objs) {
					if (selectObjectImpl(mouse, objs)) {
						m_draggingObject = true;
						return BREAK_FROM_LOOP;
					} else {
						return STAY_IN_LOOP;
					}
				}, layer);
			}
		private:
			bool editPolygon(int layer, SDL_Point mouse) {
				if (ImGui::IsKeyDown(ImGuiKey_F5)) {
					m_polygonBuilder.building = true;
				}
				if (m_polygonBuilder.building) {
					auto polygon = m_polygonBuilder(mouse);
					if (polygon) {
						auto& polygons = std::get<EditedObjectHive<Polygon>>(this->layers[layer]);
						auto insertedPolygonIt = polygons.insert(std::move(*polygon));
						m_selectedObject = SelectedObjectData{ &(*insertedPolygonIt), &polygons, insertedPolygonIt };
						return false;
					} 
				}
				return m_polygonBuilder.building;
			}
		public:
			void operator()(int layer, SDL_Point mouse, SDL_Renderer* renderer) {
				if (!editPolygon(layer, mouse)) {
					selectObject(layer, mouse);
					selectiveVisit([&, this](auto& selectedObj) {
						if (selectedObj.obj != nullptr) {
							edit(mouse, selectedObj);
						}
					}, m_selectedObject);
				}
				this->forEach([&](int layer, const auto& editedObj) {
					editedObj.obj.render(renderer);
					return STAY_IN_LOOP;
				});
			}

			void makeLayerMoreVisible(int layer, uint8_t reducedOpacity) {
				auto setOpacity = [](auto& hiveTuple, uint8_t opacity) {
					forEachDataMember([&](auto& objHive) {
						for (auto& editedObj : objHive) {
							editedObj.obj.setOpacity(opacity);
						}
						return STAY_IN_LOOP;
					}, hiveTuple);
				};
				auto reduceOpacity = [&, this](auto startIt, auto endIt) {
					for (auto it = startIt; it != endIt; it++) {
						auto& objects = it->second;
						setOpacity(objects, reducedOpacity);
					}
				};
				this->layers[layer]; //insert layer if it doesn't already exist
				auto visibleLayerIt = this->layers.find(layer);
				
				setOpacity(visibleLayerIt->second, 255);
				reduceOpacity(this->layers.begin(), visibleLayerIt);
				reduceOpacity(std::next(visibleLayerIt), this->layers.end());
			}

			void deselectSelectedObject() noexcept {
				m_selectedObject = std::monostate{};
				m_draggingObject = false;
			}
		};
	}
}