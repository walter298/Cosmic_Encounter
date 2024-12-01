#include "SpriteEditor.h"

#include <fstream>

#include "../data_util/BasicJsonSerialization.h"
#include "../data_util/File.h"
#include "../Sprite.h"

using nv::editor::SpriteEditor;

void SpriteEditor::open(SDL_Renderer* renderer) {
	auto filePath = openFile({ {"Sprites", "nv_sprite"} });
	if (filePath) {
		try {
			m_objects.clear();

			std::ifstream file{ *filePath };
			auto json = json::parse(file);
			auto texJsonData = json.get<Sprite::JsonFormat>();
			m_objects.reserve(texJsonData.size());

			for (auto& [layer, texObjs] : texJsonData) {
				auto& currTexLayer = std::get<EditedObjectHive<Texture>>(m_objects.layers[layer]);
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
	auto filename = saveNewFile({ { "Sprites", "nv_sprite" } }, [this](auto path) {
		Sprite::JsonFormat texObjsJson;
		for (const auto& [layer, texObjs] : m_objects.layers) {
			for (const auto& editedObj : std::get<EditedObjectHive<Texture>>(texObjs)) {
				texObjsJson[layer].emplace_back(editedObj.obj.getTexPath(), editedObj.obj.texData);
			}
		}

		json json;
		json["texture_object_layers"] = std::move(texObjsJson);
		
		return json.dump(2);
	});
}

void nv::editor::SpriteEditor::saveAsTextureObject() {
	auto& currTexLayer = std::get<EditedObjectHive<Texture>>(m_objects[m_currLayer]);
	if (currTexLayer.size() != 1) {
		std::println("Error: to save as texture object, there must be exactly one texture object in the current layer");
		return;
	}
	
	saveNewFile({ { "Texture Objects", "nv_texture_object" } }, [&, this](auto path) {
		json json;
		currTexLayer.begin()->obj.save(json);
		return json.dump(2);
	});
}

void nv::editor::SpriteEditor::setIdenticalLayout() {
	auto& texsToMove = std::get<EditedObjectHive<Texture>>(m_objects[m_currLayer]);
	auto& targetTexs = std::get<EditedObjectHive<Texture>>(m_objects[m_currLayoutLayer]);

	if (texsToMove.size() != targetTexs.size()) {
		std::println("Error: cannot model layout of layers with different # of textures");
		return;
	}
	for (auto [texToMove, targetTex] : views::zip(texsToMove, targetTexs)) {
		texToMove.obj.setPos(targetTex.obj.getPos());
	}
}

void SpriteEditor::showSpriteOptions(SDL_Renderer* renderer, SDL_Point mouse) {
	static constexpr ImVec2 layerOptionPos{ 0.0f, 0.0f };
	static constexpr ImVec2 layerOptionsSize{ 400.0f, 300.0f };

	ImGui::SetNextWindowPos(layerOptionPos);
	ImGui::SetNextWindowSize(layerOptionsSize);

	ImGui::Begin("Layer");

	//select layer
	if (ImGui::InputInt("Layer", &m_currLayer)) {
		m_objects.makeLayerMoreVisible(m_currLayer, 30);
	}

	//insert textures
	if (ImGui::Button("Create Texture(s)")) {
		m_objects.addFromFiles<Texture>(m_currLayer, renderer, mouse);
	}

	ImGui::SameLine();
	if (ImGui::Button("Load Spritesheet")) {

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
	: m_renderer{ renderer }, m_objects{ renderer }
{
	m_objects.layers[m_currLayer];
}

nv::editor::EditorDest SpriteEditor::imguiRender() {
	auto mouse = convertPair<SDL_Point>(ImGui::GetMousePos());

	m_objects(m_currLayer, mouse, m_renderer);
	showSpriteOptions(m_renderer, mouse);

	return EditorDest::None;
}

void nv::editor::SpriteEditor::sdlRender() const noexcept {
	m_objects.forEach([this](int layer, const auto& obj) {
		obj.obj.render(m_renderer);
		return STAY_IN_LOOP;
	});
}
