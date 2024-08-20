#include "SpriteEditor.h"

#include <fstream>

#include "data_util/BasicJsonSerialization.h"
#include "Sprite.h"

using nv::editor::SpriteEditor;

void SpriteEditor::reduceOpacityOfOtherLayers() {
	auto reduceOpacityImpl = [](auto&& objLayers) {
		for (auto& [layer, objLayer] : objLayers) {
			for (auto& editedObj : objLayer) {
				editedObj.obj.setOpacity(100);
			}
		}
	};

	auto visibleLayerIt = m_texLayers.find(m_currLayer); //call find to get iterator
	for (auto& editedObj : visibleLayerIt->second) {
		editedObj.obj.setOpacity(255);
	}

	auto beforeVisibleLayer = ranges::subrange(m_texLayers.begin(), visibleLayerIt);
	auto afterVisibleLayer = ranges::subrange(std::next(visibleLayerIt), m_texLayers.end());
	reduceOpacityImpl(beforeVisibleLayer);
	reduceOpacityImpl(afterVisibleLayer);
}

void SpriteEditor::open(SDL_Renderer* renderer) {
	auto filePath = openFilePath();
	if (filePath) {
		try {
			m_texLayers.clear();

			std::ifstream file{ *filePath };
			auto json = json::parse(file);
			auto texJsonData = json.get<Sprite::JsonFormat>();
			m_texLayers.reserve(texJsonData.size());

			for (auto& [layer, texObjs] : texJsonData) {
				auto& currTexLayer = m_texLayers[layer];
				currTexLayer.reserve(texObjs.size());
				for (auto& [texPath, texData] : texObjs) {
					currTexLayer.emplace(renderer, texPath, loadSharedTexture(renderer, texPath), std::move(texData));
				}
			}
		} catch (json::exception e) {
			std::println("{}", e.what());
		}
	}
}

void nv::editor::SpriteEditor::save() {
	auto filename = saveFile(L"Save Texture");
	if (!filename) {
		return;
	}

	Sprite::JsonFormat texObjsJson;
	for (const auto& [layer, texObjs] : m_texLayers) {
		for (const auto& editedObj : texObjs) {
			texObjsJson[layer].emplace_back(editedObj.obj.getTexPath(), editedObj.obj.texData);
		}
	}

	json json;
	json["texture_object_layers"] = std::move(texObjsJson);

	//write json
	std::ofstream file{ *filename };
	assert(file.is_open());
	file << json.dump(2);
	file.close();
}

void nv::editor::SpriteEditor::saveAsTextureObject() {
	auto& currTexLayer = m_texLayers[m_currLayer];
	if (currTexLayer.size() != 1) {
		std::println("Error: to save as texture object, there must be exactly one texture object in the current layer");
		return;
	}
	auto filename = saveFile(L"Save as .nv_texture_object");
	if (!filename) {
		return;
	}
	json json;
	currTexLayer.begin()->obj.save(json);
	std::ofstream jsonFile{ *filename };
	jsonFile << json.dump(2);
}

void nv::editor::SpriteEditor::insertTextures(SDL_Renderer* renderer) {
	auto texPaths = openFilePaths();
	if (texPaths) {
		TextureData defaultPos;
		defaultPos.ren.setPos(400, 400);
		defaultPos.ren.setSize(100, 100);
		auto& currLayer = m_texLayers[m_currLayer];
		for (const auto& texPath : *texPaths) {
			currLayer.emplace(
				renderer,
				texPath, //todo: make texPath part of TextureObject constructor
				loadSharedTexture(renderer, texPath),
				defaultPos
			);
			defaultPos.ren.rect.x += 300;
		}
		//m_selectedTexObj.resetToLastElement(&currLayer);
		m_isTexSelected = true;
	}
}

void nv::editor::SpriteEditor::setIdenticalLayout() {
	auto& texsToMove = m_texLayers[m_currLayer];
	auto& targetTexs = m_texLayers[m_currLayoutLayer];

	if (texsToMove.size() != targetTexs.size()) {
		std::println("Error: cannot model layout of layers with different # of textures");
		return;
	}
	for (auto [texToMove, targetTex] : views::zip(texsToMove, targetTexs)) {
		texToMove.obj.setPos(targetTex.obj.getPos());
	}
}

void SpriteEditor::showSpriteOptions(SDL_Renderer* renderer) {
	static constexpr ImVec2 layerOptionPos{ 0.0f, 0.0f };
	static constexpr ImVec2 layerOptionsSize{ 200.0f, 200.0f };

	ImGui::SetNextWindowPos(layerOptionPos);
	ImGui::SetNextWindowSize(layerOptionsSize);

	ImGui::Begin("Layer");

	//select layer
	if (ImGui::InputInt("Layer", &m_currLayer)) {
		auto tupleWrapper = std::tie(m_texLayers);
		//makeOneLayerMoreVisible(std::tie(m_texLayers), m_currLayer, 100);
	}

	//insert textures
	if (ImGui::Button("Insert Texture(s)")) {
		insertTextures(renderer);
	}

	if (ImGui::Button("Set Layout")) {
		setIdenticalLayout();
	}
	ImGui::SameLine();
	ImGui::InputInt("Target Layer", &m_currLayoutLayer);
	
	if (ImGui::Button("Save")) {
		save();
	}
	if (ImGui::Button("Save as Texture Object")) {
		saveAsTextureObject();
	}
	ImGui::SameLine();
	if (ImGui::Button("Open")) {
		open(renderer);
	}
	ImGui::End();
}

SpriteEditor::SpriteEditor(SDL_Renderer* renderer) noexcept
	: m_renderer{ renderer } //m_texDataEditor { { 0, 500 } }
{
}

nv::editor::EditorDest SpriteEditor::imguiRender() {
	showSpriteOptions(m_renderer);
	auto& currLayer = m_texLayers[m_currLayer];
	if (currLayer.empty()) {
		return EditorDest::None;
	}
	if (m_isTexSelected) {
		edit(m_selectedTexObj);
		if (m_selectedTexObj.obj == nullptr) { //if we deleted a texture object
			m_isTexSelected = false;
		}
	} 
	if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
		auto selectedObj = selectObj(currLayer, convertPair<SDL_Point>(ImGui::GetMousePos()));
		if (selectedObj != currLayer.end()) {
			m_selectedTexObj.obj      = &(*selectedObj);
			m_selectedTexObj.objLayer = &currLayer;
			m_selectedTexObj.it       = selectedObj;
			m_isTexSelected = true;
		} 
	}
	return EditorDest::None;
}

void nv::editor::SpriteEditor::sdlRender() const noexcept {
	for (const auto& [layer, textures] : m_texLayers) {
		for (const auto& editedTex : textures) {
			editedTex.obj.render();
		}
	}
}
