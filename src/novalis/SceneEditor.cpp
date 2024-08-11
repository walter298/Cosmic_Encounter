#include "SceneEditor.h"

using nv::editor::SceneEditor;

void SceneEditor::loadSprite() {
	auto filePath = openFilePath();
	if (!filePath) {
		return;
	}
	try {
		std::ifstream file{ *filePath };
		auto json = json::parse(file);
		auto& currSpriteLayer = std::get<EditedObjectVector<Sprite>>(m_objectLayers[m_currLayer]);
		currSpriteLayer.emplace_back(m_renderer, json, m_texMap);
		m_selectedSpriteData.resetToLastElement(&currSpriteLayer);
	} catch (json::exception e) {
		std::println("{}", e.what());
	}
}

void SceneEditor::createRect() {
	auto& rectLayer = std::get<EditedObjectVector<Rect>>(m_objectLayers[m_currLayer]);
	auto& lastRect = rectLayer.emplace_back(m_renderer, 0, 0, 0, 0).obj;
	lastRect.setPos(NV_SCREEN_WIDTH / 2, NV_SCREEN_HEIGHT / 2);
	lastRect.setSize(200, 100);
	m_showingRightClickOptions = false;
	m_selectedRectData.resetToLastElement(&rectLayer);
}

void SceneEditor::reduceOpacityOfOtherLayers() {
	auto reduceOpacityImpl = [](auto&& objLayers) {
		for (auto& [layer, objLayer] : objLayers) {
			forEachDataMember([](auto& objs) {
				for (auto& editedObj : objs) {
					editedObj.obj.setOpacity(100);
				}
				return STAY_IN_LOOP;
			}, objLayer);
		}
	};

	//set to full opacity in case it was already reduced before we called this function
	forEachDataMember([](auto& objs) {
		for (auto& editedObj : objs) {
			editedObj.obj.setOpacity(255);
		}
		return STAY_IN_LOOP;
	}, m_objectLayers[m_currLayer]);

	auto visibleLayerIt = m_objectLayers.find(m_currLayer); //call find to get iterator
	auto beforeVisibleLayer = ranges::subrange(m_objectLayers.begin(), visibleLayerIt);
	auto afterVisibleLayer = ranges::subrange(std::next(visibleLayerIt), m_objectLayers.end());
	reduceOpacityImpl(beforeVisibleLayer);
	reduceOpacityImpl(afterVisibleLayer);
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
		auto path = openFilePath();
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

	if (selectedFont != nullptr) {
		auto& currLayer = std::get<EditedObjectVector<Text>>(m_objectLayers[m_currLayer]);
		currLayer.emplace_back(m_renderer, "generic text"sv, m_fontPath, m_fontSize, selectedFont);
		auto& insertedTex = currLayer.back();
		insertedTex.obj.setPos(NV_SCREEN_WIDTH / 2, NV_SCREEN_HEIGHT / 2);
		m_showingFontOptions = false;
		m_selectedTextData.resetToLastElement(&currLayer);
	}

	ImGui::End();
}

void SceneEditor::createTextures() noexcept {
	auto texPaths = openFilePaths();
	if (!texPaths) {
		return;
	}
	auto& texLayer = std::get<EditedObjectVector<Texture>>(m_objectLayers[m_currLayer]);
	for (auto& texPath : *texPaths) {
		texLayer.emplace_back(
			m_renderer,
			texPath,
			loadSharedTexture(m_renderer, texPath),
			TextureData{}
		);
		convertFullToRegularPath(texPath);
		texLayer.back().obj.setPos(NV_SCREEN_WIDTH / 2, NV_SCREEN_HEIGHT / 2);
		texLayer.back().obj.scale(200, 200);
		texLayer.back().width  = 200;
		texLayer.back().height = 200;
		texLayer.back().name   = fileName(texPath);
	}
	m_selectedTextureData.resetToLastElement(&texLayer);
}

void SceneEditor::showRightClickOptions() noexcept {
	static constexpr ImVec2 btnSize{ 210.0f, 60.0f };
	static constexpr auto winSize = buttonList(btnSize, 4);

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
	if (ImGui::Button("Create Text", btnSize)) {
		showFontOptions();
		m_showingRightClickOptions = false;
		m_showingFontOptions = true;
	}
	if (ImGui::Button("Create Rect", btnSize)) {
		createRect();
	}
	ImGui::End();
}

void nv::editor::SceneEditor::save() const noexcept {
	auto filename = saveFile(L"Save File");
	if (!filename) {
		return;
	}

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
	for (const auto& [layer, objs] : m_objectLayers) {
		auto& newLayerJson = objectsRoot.emplace_back();
		newLayerJson["layer"] = layer;

		forEachDataMember([&](const auto& objs) {
			saveObjectsImpl(newLayerJson, objs);
			return STAY_IN_LOOP;
		}, objs);
	}

	//write json to the file
	std::ofstream file{ *filename };
	file << root.dump(2);
	file.close();
}

void nv::editor::SceneEditor::showSceneOptions() noexcept {
	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize({ 150, 100 });
	ImGui::Begin("Scene");
	if (ImGui::InputInt("Layer", &m_currLayer)) {
		//make sure that there are no pointers to previous objects in other layers in selected object data
		auto tiedSelectedObjs = std::tie(m_selectedSpriteData, m_selectedTextureData, m_selectedTextData, m_selectedRectData);
		forEachDataMember([this](auto& selectedObj) {
			selectedObj.reset();
			return STAY_IN_LOOP;
		}, tiedSelectedObjs);
		m_selectedObjType = SelectedObjectType::None; //we can't have a selected object in a different layer

		reduceOpacityOfOtherLayers();
	}
	if (ImGui::Button("Save")) {
		save();
	}
	ImGui::End();
}

void nv::editor::SceneEditor::editLayers() {
	ImGui::SetNextWindowSize({ 300, 220 });
	ImGui::SetNextWindowPos({ 0, 160 });

	auto editImpl = [this](std::string_view name, auto& objData) {
		ImGui::Begin(name.data());
		edit(objData);
		ImGui::End();
		if (objData.obj == nullptr) {
			m_selectedObjType = SelectedObjectType::None;
		}
	};

	switch (m_selectedObjType) {
	case SelectedObjectType::Sprite:
		editImpl("Sprite", m_selectedSpriteData);
		break;
	case SelectedObjectType::Texture:
		editImpl("Texture Object", m_selectedTextureData);
		break;
	case SelectedObjectType::Text:
		editImpl("Text", m_selectedTextData);
		break;
	case SelectedObjectType::Rect:
		editImpl("Rect", m_selectedRectData);
		break;
	}

	auto tiedObjData = std::tie(m_selectedSpriteData, m_selectedTextureData, m_selectedTextData, m_selectedRectData);
	forEachDataMember([&, this](auto& objs, auto& selectedObjData) {
		selectImpl(objs, selectedObjData);
		return STAY_IN_LOOP;
	}, m_objectLayers[m_currLayer], tiedObjData);
}

SceneEditor::SceneEditor(SDL_Renderer* renderer)
	: m_renderer{ renderer }
{ 
	m_selectedObjMap.get<Sprite>()  = SelectedObjectType::Sprite;
	m_selectedObjMap.get<Texture>() = SelectedObjectType::Texture;
	m_selectedObjMap.get<Text>()    = SelectedObjectType::Text;
	m_selectedObjMap.get<Rect>()    = SelectedObjectType::Rect;
}

nv::editor::EditorDest SceneEditor::imguiRender() {
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
		cameraMoveEditedObjects(-5, 0, m_objectLayers);
	}
	if (ImGui::IsKeyDown(ImGuiKey_RightArrow)) {
		cameraMoveEditedObjects(5, 0, m_objectLayers);
	}
	if (ImGui::IsKeyDown(ImGuiKey_UpArrow)) {
		cameraMoveEditedObjects(0, -5, m_objectLayers);
	}
	if (ImGui::IsKeyDown(ImGuiKey_DownArrow)) {
		cameraMoveEditedObjects(0, 5, m_objectLayers);
	}
	editLayers();
	return EditorDest::None;
}

void nv::editor::SceneEditor::sdlRender() const noexcept {
	for (const auto& [layer, objLayer] : m_objectLayers) {
		forEachDataMember([](const auto& objLayer) {
			for (const auto& editedObj : objLayer) {
				editedObj.obj.render();
			}
			return STAY_IN_LOOP;
		}, objLayer);
	}
}
