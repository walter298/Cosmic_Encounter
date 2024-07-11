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
		auto& currSpriteLayer = m_spriteLayers[m_currLayer];
		currSpriteLayer.emplace_back(m_renderer, json, m_texMap);
		m_selectedSpriteData.resetToLastElement(&currSpriteLayer);
	} catch (json::exception e) {
		std::println("{}", e.what());
	}
}

void SceneEditor::createRect() {
	auto& currRectLayer = m_rectLayers[m_currLayer];
	currRectLayer.emplace_back(m_renderer);
	auto& lastRect = currRectLayer.back().obj;
	lastRect.setPos(NV_SCREEN_WIDTH / 2, NV_SCREEN_HEIGHT / 2);
	lastRect.setSize(200, 100);
	m_showingRightClickOptions = false;
	m_selectedRectData.resetToLastElement(&currRectLayer);
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
			auto loadedFont = loadFont(m_fontPath, m_fontSize);
			if (loadedFont == nullptr) {
				std::println("{}", TTF_GetError());
			} else {
				selectedFont = loadedFont.get();
				m_fontMap.emplace(m_fontPath, std::move(loadedFont));
			}
		}
	}

	if (selectedFont != nullptr) {
		auto& currLayer = m_textLayers[m_currLayer];
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
	auto& texLayer = m_texObjLayers[m_currLayer];
	for (const auto& texPath : *texPaths) {
		texLayer.emplace_back(
			m_renderer,
			texPath,
			loadSharedTexture(m_renderer, texPath),
			TextureData{}
		);
		texLayer.back().obj.setPos(NV_SCREEN_WIDTH / 2, NV_SCREEN_HEIGHT / 2);
		texLayer.back().obj.scale(200, 200);
		texLayer.back().width  = 200;
		texLayer.back().height = 200;
		texLayer.back().name   = fileName(texPath);
	}
	m_selectedTexObjData.resetToLastElement(&texLayer);
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

	auto spritesJson = json::array();
	auto texObjsJson = json::array();
	auto textJson    = json::array();
	auto rectsJson   = json::array();

	saveObjects(m_spriteLayers, spritesJson);
	saveObjects(m_texObjLayers, texObjsJson);
	saveObjects(m_textLayers, textJson);
	saveObjects(m_rectLayers, rectsJson);

	root["sprites"]         = std::move(spritesJson);
	root["texture_objects"] = std::move(texObjsJson);
	root["text"]            = std::move(textJson);
	root["rects"]           = std::move(rectsJson);

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
		makeOneLayerMoreVisible(m_spriteLayers, m_currLayer, 50);
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
		editImpl("Texture Object", m_selectedTexObjData);
		break;
	case SelectedObjectType::Text:
		editImpl("Text", m_selectedTextData);
		break;
	case SelectedObjectType::Rect:
		editImpl("Rect", m_selectedRectData);
		break;
	}

	auto select = [this](auto& objLayer, auto& selectedObjData, SelectedObjectType objType) {
		if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
			auto selectedObj = selectObj(objLayer, convertPair<SDL_Point>(ImGui::GetMousePos()));
			if (selectedObj != objLayer.end()) {
				selectedObjData.obj = &(*selectedObj);
				selectedObjData.objLayer = &objLayer;
				selectedObjData.it = selectedObj;
				m_selectedObjType = objType;
			}
		}
	};
	select(m_spriteLayers[m_currLayer], m_selectedSpriteData, SelectedObjectType::Sprite);
	select(m_texObjLayers[m_currLayer], m_selectedTexObjData, SelectedObjectType::Texture);
	select(m_textLayers[m_currLayer], m_selectedTextData, SelectedObjectType::Text);
	select(m_rectLayers[m_currLayer], m_selectedRectData, SelectedObjectType::Rect);
}

SceneEditor::SceneEditor(SDL_Renderer* renderer)
	: m_renderer{ renderer }
{ 
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
		cameraMove(-5, 0, m_spriteLayers, m_texObjLayers);
	}
	if (ImGui::IsKeyDown(ImGuiKey_RightArrow)) {
		cameraMove(5, 0, m_spriteLayers, m_texObjLayers);
	}
	if (ImGui::IsKeyDown(ImGuiKey_UpArrow)) {
		cameraMove(0, -5, m_spriteLayers, m_texObjLayers);
	}
	if (ImGui::IsKeyDown(ImGuiKey_DownArrow)) {
		cameraMove(0, 5, m_spriteLayers, m_texObjLayers);
	}
	editLayers();
	return EditorDest::None;
}

void nv::editor::SceneEditor::sdlRender() const noexcept {
	renderCopy(m_texObjLayers, m_spriteLayers, m_rectLayers, m_textLayers);
}
