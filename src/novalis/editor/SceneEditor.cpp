#include "SceneEditor.h"

#include <SDL2/SDL2_gfxPrimitives.h>

#include "../data_util/BasicJsonSerialization.h"
#include "../data_util/File.h"

using nv::editor::SceneEditor;

void SceneEditor::loadSprite() {
	/*auto filePath = openFile({});
	if (!filePath) {
		return;
	}
	try {
		std::ifstream file{ *filePath };
		auto json = json::parse(file);
		auto& currSpriteLayer = std::get<EditedObjectHive<Sprite>>(m_objects.layers[m_currLayer]);
		currSpriteLayer.emplace(m_renderer, json, m_texMap);
	} catch (json::exception e) {
		std::println("{}", e.what());
	}*/
}

void SceneEditor::createRect() {
	auto& rectLayer = std::get<EditedObjectHive<Rect>>(m_objects.layers[m_currLayer]);
	rectLayer.emplace(400, 400, 200, 100);
	m_showingRightClickOptions = false;
}

void SceneEditor::reduceOpacityOfOtherLayers() {
	auto reduceOpacityImpl = [](auto&& objLayers) {
		/*for (auto& [layer, objLayer] : objLayers) {
			forEachDataMember([](auto& objs) {
				for (auto& editedObj : objs) {
					editedObj.obj.setOpacity(100);
				}
				return STAY_IN_LOOP;
			}, objLayer);
		}*/
	};

	//set to full opacity in case it was already reduced before we called this function
	/*forEachDataMember([](auto& objs) {
		for (auto& editedObj : objs) {
			editedObj.obj.setOpacity(255);
		}
		return STAY_IN_LOOP;
	}, m_objects.layers[m_currLayer]);*/

	auto visibleLayerIt = m_objects.layers.find(m_currLayer); //call find to get iterator
	/*auto beforeVisibleLayer = ranges::subrange(m_objects.layers.begin(), visibleLayerIt);
	auto afterVisibleLayer = ranges::subrange(std::next(visibleLayerIt), m_objects.layers.end());
	reduceOpacityImpl(beforeVisibleLayer);
	reduceOpacityImpl(afterVisibleLayer);*/
}

void nv::editor::SceneEditor::selectSpecialPoint() noexcept {
	if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
		auto mouse = convertPair<SDL_Point>(ImGui::GetMousePos());
		auto selectedPointIt = ranges::find_if(m_specialPoints, [&](const auto& point) {
			return point.containsCoord(mouse);
		});
		if (selectedPointIt != m_specialPoints.end()) {
			/*m_draggingObject = true;
			m_selectedSpecialPoint = &(*selectedPointIt);
			m_selectedObjType = SelectedObjectType::SpecialPoint;*/
		}
	}
}

void nv::editor::SceneEditor::editSelectedSpecialPoint() {
	//ScopeExit exit{ [] { ImGui::End(); } };

	//ImGui::SetNextWindowSize({ 250, 120 });
	//ImGui::SetNextWindowPos({ 0, 480 });

	//ImGui::Begin("Special Point");
	//ImGui::InputText("name", &m_selectedSpecialPoint->name);
	//if (ImGui::Button("Delete")) {
	//	/*auto pointIt = ranges::find_if(m_specialPoints, [this](const auto& point) {
	//		return point.name == m_selectedSpecialPoint->name;
	//	});
	//	m_specialPoints.erase(pointIt);
	//	m_selectedSpecialPoint = nullptr;
	//	m_selectedObjType      = SelectedObjectType::None;*/
	//	return;
	//}
	//auto mouseDelta = convertPair<SDL_Point>(ImGui::GetMouseDragDelta());
	//if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && m_draggingObject) {
	//	m_selectedSpecialPoint->point.x += mouseDelta.x;
	//	m_selectedSpecialPoint->point.y += mouseDelta.y;
	//}

	//ImGui::ResetMouseDragDelta();
}

void nv::editor::SceneEditor::createSpecialPoint() {
	/*auto mouse = convertPair<SDL_Point>(ImGui::GetMousePos());
	m_selectedSpecialPoint = &m_specialPoints.emplace_back(mouse, "special_point");
	m_selectedObjType = SelectedObjectType::SpecialPoint;*/
}

void SceneEditor::showFontOptions() {
	ImGui::SetNextWindowSize({ 250, 120 });
	ImGui::SetNextWindowPos({ 0, 480 });

	ImGui::Begin("Font Options");

	TTF_Font* selectedFont = nullptr;

	for (const auto& [path, font] : m_fontMap) {
		auto filename = fileName(path);
		if (ImGui::Button(filename.data())) {
			selectedFont = font.get();
		}
	}
	ImGui::InputInt("Font Size", &m_fontSize);

	if (selectedFont == nullptr && ImGui::Button("Open Font")) {
		auto path = openFile({ { "Fonts", "TTF" } });
		if (path) { 
			m_fontPath = *path;

			convertFullToRegularPath(m_fontPath);

			if (m_fontMap.contains(m_fontPath)) { //loading a duplicate font would invalidate pointers to original font
				std::println("Error: font map already contains {}", m_fontPath);
				return;
			}
			auto loadedFont = loadFont(*path, m_fontSize); //load with *path because we need the full path
			if (loadedFont == nullptr) {
				std::println("{}", TTF_GetError());
			} else {
				selectedFont = loadedFont.get();
				m_fontMap.emplace(m_fontPath + std::to_string(m_fontSize), std::move(loadedFont));
			}
		}
	}

	using namespace std::literals;

	if (selectedFont != nullptr) {
		auto& currLayer = std::get<EditedObjectHive<Text>>(m_objects.layers[m_currLayer]);
		auto texIt = currLayer.emplace(m_renderer, "generic text"sv, m_fontPath, m_fontSize, selectedFont);
		auto& insertedTex = *texIt;
		insertedTex.obj.setPos(150, 300);
		m_showingFontOptions = false;
	}

	ImGui::End();
}

void SceneEditor::createTextures() noexcept {
	auto texPaths = openMultipleFiles({});
	if (!texPaths) {
		return;
	}
	//auto& texLayer = std::get<EditedObjectHive<Texture>>(m_objects.layers[m_currLayer]);
	
	int currX   = 0;
	int currY   = 0;
	int rowC    = 0;
	int columnC = 0;
	auto updateRowOrColumn = [](int& rowOrColumnC, int totalRowsOrColumns, int& currCoord, int deltaCoord) {
		if (rowOrColumnC == totalRowsOrColumns) {
			rowOrColumnC = 0;
			currCoord = 0;
		} else {
			rowOrColumnC++;
			currCoord += deltaCoord;
		}
	};

	m_textureRowC    = m_textureRowC < 1 ? 1 : m_textureRowC;
	m_textureColumnC = m_textureRowC < 1 ? 1 : m_textureRowC;

	for (auto& texPath : *texPaths) {
		//load texture and get width and height
		auto loadedTex = loadSharedTexture(m_renderer, texPath);
		int initialTexWidth  = 0;
		int initialTexHeight = 0;
		SDL_QueryTexture(loadedTex.get(), nullptr, nullptr, &initialTexWidth, &initialTexHeight);
		
		auto& texLayer = std::get<EditedObjectHive<Texture>>(m_objects.layers[m_currLayer]);

		//put newly loaded texture into the scene
		auto it = texLayer.emplace(
			m_renderer,
			texPath,
			std::move(loadedTex),
			TextureData{}
		);

		auto& editedObj = *it;
		editedObj.obj.setPos(currX, currY);
		editedObj.obj.setSize(initialTexWidth, initialTexHeight);
		editedObj.width  = initialTexWidth;
		editedObj.height = initialTexHeight;
		editedObj.name   = fileName(texPath);
		
		if (rowC == m_textureRowC) {
			rowC = 0;
			currX = 0;
		} else {
			rowC++;
			currX += initialTexWidth;
		}
		if (columnC == m_textureColumnC) {
			columnC = 0;
			currY += initialTexHeight;
		} else {
			columnC++;
		}
	}
}

void SceneEditor::showRightClickOptions() noexcept {
	static constexpr ImVec2 btnSize{ 210.0f, 60.0f };
	static constexpr auto winSize = buttonList(btnSize, 5);

	ImGui::SetNextWindowPos(m_rightClickWinPos);
	ImGui::SetNextWindowSize(winSize);
	ImGui::Begin("Options");
	if (ImGui::Button("Upload Sprite", btnSize)) {
		loadSprite();
		m_showingRightClickOptions = false;
	}
	if (ImGui::Button("Create Texture", btnSize)) {
		createTextures();
		m_showingRightClickOptions = false;
	}
	ImGui::InputInt("Rows", &m_textureRowC);
	ImGui::InputInt("Columns", &m_textureColumnC);

	if (ImGui::Button("Create Text", btnSize)) {
		showFontOptions();
		m_showingRightClickOptions = false;
		m_showingFontOptions = true;
	}
	if (ImGui::Button("Create Rect", btnSize)) {
		createRect();
		m_showingRightClickOptions = false;
	}
	if (ImGui::Button("Create Special Point", btnSize)) {
		createSpecialPoint();
		m_showingRightClickOptions = false;
	}
	ImGui::End();
}

void nv::editor::SceneEditor::save() const noexcept {
	saveNewFile({ { "Scenes", "nv_scene" } }, [this](auto path) {
		json root;
		json& objectsRoot = root["objects"];

		auto saveObjectsImpl = [&](json& layerRoot, const auto& objs) {
			auto& objsJson = layerRoot[typeid(ValueType<decltype(objs)>::obj).name()];
			objsJson = json::array();
			for (const auto& editedObj : objs) {
				auto& objJson = objsJson.emplace_back();
				editedObj.obj.save(objJson);
				objJson["name"] = editedObj.name;
			}
		};
		for (const auto& [layer, objs] : m_objects.layers) {
			auto& newLayerJson = objectsRoot.emplace_back();
			newLayerJson["layer"] = layer;

			forEachDataMember([&](const auto& objs) {
				saveObjectsImpl(newLayerJson, objs);
				return STAY_IN_LOOP;
			}, objs);
		}

		//write special points
		auto& specialPointsJson = root["special_points"];
		specialPointsJson = json::array();
		for (auto point : m_specialPoints) {
			auto& pointJson = specialPointsJson.emplace_back();
			pointJson["name"] = point.name;
			pointJson["point"] = point.point;
		}
		return root.dump(2);
	});
}

void nv::editor::SceneEditor::showSceneOptions() noexcept {
	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize({ 150, 100 });
	ImGui::Begin("Scene");
	
	if (ImGui::InputInt("Layer", &m_currLayer)) {
		m_objects.deselectSelectedObject();
		m_objects.makeLayerMoreVisible(m_currLayer, 100);
	}
	if (ImGui::Button("Save")) {
		save();
	}
	ImGui::End();
}

void nv::editor::SceneEditor::cameraMove(int dx, int dy) {
	m_objects.forEach([&](int layer, auto& obj) {
		//obj.obj.move(dx, dy);
		return STAY_IN_LOOP;
	});
}

void nv::editor::SceneEditor::editLayers() {
	//auto editImpl = [this](std::string_view name, auto& objData) {
	//	ImGui::SetNextWindowSize({ 300, 220 });
	//	ImGui::SetNextWindowPos({ 0, 160 });

	//	ImGui::Begin(name.data());
	//	edit(objData);
	//	ImGui::End();
	//	if (objData.obj == nullptr) {
	//		m_selectedObjType = SelectedObjectType::None;
	//	}
	//};

	//switch (m_selectedObjType) {
	//case SelectedObjectType::Sprite:
	//	editImpl("Sprite", m_selectedSpriteData);
	//	break;
	//case SelectedObjectType::Texture:
	//	editImpl("Texture Object", m_selectedTextureData);
	//	break;
	//case SelectedObjectType::Text:
	//	editImpl("Text", m_selectedTextData);
	//	break;
	//case SelectedObjectType::Rect:
	//	editImpl("Rect", m_selectedRectData);
	//	break;
	//case SelectedObjectType::SpecialPoint:
	//	editSelectedSpecialPoint();
	//	break;
	//}

	////select objects
	//if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
	//	m_draggingObject = false;
	//	return;
	///*if we have not released the button and we are dragging an object, then we don't need
	//to select a new object*/
	//} else if (m_draggingObject) {
	//	return;
	//}
	//auto tiedObjData = std::tie(m_selectedSpriteData, m_selectedTextureData, m_selectedTextData, m_selectedRectData);
	//forEachDataMember([&, this](auto& objs, auto& selectedObjData) {
	//	if (selectImpl(objs, selectedObjData)) {
	//		m_draggingObject = true;
	//		return BREAK_FROM_LOOP;
	//	} else {
	//		return STAY_IN_LOOP;
	//	}
	//}, m_objects[m_currLayer], tiedObjData);
	//selectSpecialPoint();
	//m_objectSelection(m_objects);
}

SceneEditor::SceneEditor(SDL_Renderer* renderer)
	: m_renderer{ renderer }, m_objects{ renderer }
{ 
}

nv::editor::EditorDest SceneEditor::imguiRender() {
	auto mouse = convertPair<SDL_Point>(ImGui::GetMousePos());

	m_objects(m_currLayer, mouse, m_renderer);

	showSceneOptions();
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
		m_showingRightClickOptions = true;
		m_rightClickWinPos = ImGui::GetMousePos();
	}
	if (m_showingRightClickOptions) {
		showRightClickOptions();
	}
	if (m_showingFontOptions) {
		showFontOptions();
	}
	if (ImGui::IsKeyDown(ImGuiKey_LeftArrow)) {
		cameraMove(7, 0);
	}
	if (ImGui::IsKeyDown(ImGuiKey_RightArrow)) {
		cameraMove(-7, 0);
	}
	if (ImGui::IsKeyDown(ImGuiKey_UpArrow)) {
		cameraMove(0, 7);
	}
	if (ImGui::IsKeyDown(ImGuiKey_DownArrow)) {
		cameraMove(0, -7);
	}
	editLayers();
	return EditorDest::None;
}

void nv::editor::SceneEditor::sdlRender() const noexcept {
	m_objects.forEach([](int layer, const auto& obj) {
		//obj.obj.render();
		return STAY_IN_LOOP;
	});
	for (const auto& point : m_specialPoints) {
		point.render(m_renderer);
	}
}