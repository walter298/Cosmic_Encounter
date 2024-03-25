#include "SceneEditor.h"

using nv::editor::SceneEditor;

void SceneEditor::setBackground(EditorRenderer& renderer) {
	static constexpr ImVec2 setBackgroundBtnPos{ 0.0f, 0.0f };
	static constexpr ImVec2 setBackgroundBtnSize{ 130.0f, 50.0f };

	if (ImGui::Button("Set Background", setBackgroundBtnSize)) {
		auto backgroundPath = openFilePath();
		if (backgroundPath) {
			try {
				Background newBackground{ renderer.get(), *backgroundPath };
				m_background = std::move(newBackground);
			}
			catch (nlohmann::json::exception e) {
				std::cerr << e.what() << '\n';
			}
		}
	}
}

void SceneEditor::editBackground() {
	ImGui::SliderInt("width", &m_background.m_width, 0, NV_SCREEN_WIDTH);
	ImGui::SliderInt("height", &m_background.m_height, 0, NV_SCREEN_HEIGHT);
}

void SceneEditor::showBackgroundOptions(EditorRenderer& renderer) {
	ImGui::SetNextWindowPos(backgroundOptionsWinPos);
	ImGui::SetNextWindowSize(backgroundOptionsWinSize);

	ImGui::Begin("Background");
	setBackground(renderer);
	editBackground();
	ImGui::End();
}

void SceneEditor::resetSelectedSpriteData() noexcept {
	/*m_currSpriteLayer = 0;
	m_currSpriteSizeScale = 0;
	m_currSpriteW = 0;
	m_currSpriteH = 0;
	m_selectedSpriteData = nullptr;
	m_selectedSprite = nullptr;*/
}

void SceneEditor::saveSelectedSpriteData() noexcept {
	/*if (m_selectedSpriteData == nullptr) {
		return;
	}
	m_selectedSpriteData->scale = m_currSpriteSizeScale;
	m_selectedSpriteData->layer = m_currSpriteLayer;
	m_selectedSpriteData->originalWidth = m_currSpriteW;
	m_selectedSpriteData->originalHeight = m_currSpriteH;*/
}

void SceneEditor::insertSprite(EditorRenderer& renderer, const ImVec2& mousePos) {
	static constexpr ImVec2 spriteInsertionPos{ 350.0f, 105.0f };
	static constexpr ImVec2 spriteInsertionBtnSize{ 210.0f, 75.0f };

	auto path = openFilePath();
	if (path) {
		try {
			//add sprite to scene
			auto spriteName = fileName(*path);
			//construct sprite before we save prior sprite data so if we throw, that won't happen
			Sprite sprite{ renderer.get(), *path, spriteName };

			if (!m_sprites.empty()) {
				saveSelectedSpriteData();
				resetSelectedSpriteData();
			}

			//sprite must be INSERTED AFTER we check if we have no sprites already
			m_sprites[spriteName].insert(SpriteData{ std::move(sprite) });

			m_selectedSpriteData = &getBack(m_sprites.at(spriteName));
			m_selectedSprite = &m_selectedSpriteData->sprite;
			m_spriteRectEditor.rect = &m_selectedSprite->ren;
			m_selectedSpriteData->sprite.ren.setPos(
				static_cast<int>(mousePos.x),
				static_cast<int>(mousePos.y)
			);
			m_selectedSpriteData->originalWidth = m_selectedSprite->ren.rect.w;
			m_selectedSpriteData->originalHeight = m_selectedSprite->ren.rect.h;

			/*m_currSpriteW = m_selectedSprite->ren.rect.w;
			m_currSpriteH = m_selectedSprite->ren.rect.h;*/
			renderer.addObj(m_selectedSprite, m_selectedSpriteData->layer);
		}
		catch (nlohmann::json::exception e) {
			std::cerr << e.what() << '\n';
		}
	}
}

void SceneEditor::selectObject(const Coord& mousePos) {
	if (!ImGui::GetMouseClickedCount(ImGuiMouseButton_Left)) {
		return;
	}

	int mx = static_cast<int>(mousePos.x);
	int my = static_cast<int>(mousePos.y);

	for (auto& [name, sprites] : m_sprites) {
		for (auto& spriteData : sprites) {
			auto& [sprite, scale, layer, width, height] = spriteData;
			if (&sprite == m_selectedSprite) {
				continue;
			}
			if (sprite.ren.isCoordContained(
				static_cast<int>(mousePos.x),
				static_cast<int>(mousePos.y)))
			{
				saveSelectedSpriteData();

				m_selectedSpriteData = &spriteData;
				m_selectedSprite = &sprite;
				m_spriteRectEditor.rect = &sprite.ren;
				return;
			}
		}
	}
	for (auto& rect : m_rects) {
		if (rect.isCoordContained(mx, my)) {
			m_rectEditor.rect = &rect;
			return;
		}
	}
}

void SceneEditor::editSelectedSprite(EditorRenderer& renderer) {
	ImGui::SetNextWindowPos(spriteOptionsWinPos);
	ImGui::SetNextWindowSize(spriteOptionsWinSize);

	ImGui::Begin("Selected Sprite");

	ImGui::Text("Selected Sprite");
	if (ImGui::SliderInt("Scale", &m_selectedSpriteData->scale, 0, NV_SCREEN_WIDTH)) {
		m_selectedSprite->ren.setSize(
			m_selectedSpriteData->originalWidth + m_selectedSpriteData->scale,
			m_selectedSpriteData->originalHeight + m_selectedSpriteData->scale
		);
	}
	
	const int layerBeforeEdited = m_selectedSpriteData->layer;
	if (ImGui::InputInt("Layer", &m_selectedSpriteData->layer)) {
		renderer.removeObj(m_selectedSprite->getID(), layerBeforeEdited);
		renderer.addObj(m_selectedSprite, m_selectedSpriteData->layer);
	}

	if (ImGui::Button("Remove")) {
		renderer.removeObj(m_selectedSprite->getID(), m_selectedSpriteData->layer);
		
		const auto spriteName = m_selectedSprite->getName();
		auto spriteIt = std::ranges::find_if(
			m_sprites.at(m_selectedSprite->getName()),
			[this](const auto& spriteData) {
				return spriteData.sprite.getID() == m_selectedSprite->getID();
			}
		);
		m_selectedSpriteData = nullptr;
		m_selectedSprite = nullptr;
		m_spriteRectEditor.rect = nullptr;
		//resetSelectedSpriteData(); //MUST BE CALLED before m_selectedSprite becomes invalid
		m_sprites.at(spriteName).erase(spriteIt);
	}

	ImGui::End();
}

void SceneEditor::insertRect(EditorRenderer& renderer, const Coord& mousePos) {
	m_rects.emplace(mousePos.x, mousePos.y, 100, 100);
	m_rectEditor.rect = &getBack(m_rects);
	renderer.addRect(m_rectEditor.rect);
}

void SceneEditor::editSelectedRect(const Coord& mousePos) {
	if (m_rectEditor.rect == nullptr) {
		return;
	}

	ImGui::SetNextWindowPos(rectOptionsWinPos);
	ImGui::SetNextWindowSize(rectOptionsWinSize);
	ImGui::Begin("Rect");

	m_rectEditor.drag(mousePos);
	m_rectEditor.edit();

	ImGui::End();
}

void SceneEditor::showRightClickOptions(EditorRenderer& renderer, const Coord& mousePos) {
	static constexpr ImVec2 btnSize{ 210.0f, 60.0f };
	static constexpr auto winSize = buttonList(btnSize, 2);

	ImGui::SetNextWindowPos(m_rightClickWinPos);
	ImGui::SetNextWindowSize(winSize);
	ImGui::Begin("Options");

	if (ImGui::Button("Insert Sprite", btnSize)) {
		insertSprite(renderer, m_rightClickWinPos);
		m_showingRightClickOptions = false;
	}
	if (ImGui::Button("Create Rect", btnSize)) {
		insertRect(renderer, mousePos);
		m_showingRightClickOptions = false;
	}
	ImGui::End();
}

void SceneEditor::moveCamera(EditorRenderer& renderer) noexcept {
	static constexpr int CAMERA_DELTA = 15;
	if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
		renderer.move(-CAMERA_DELTA, 0);
		renderer.moveRects(-CAMERA_DELTA, 0);
	} else if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
		renderer.move(CAMERA_DELTA, 0);
		renderer.moveRects(CAMERA_DELTA, 0);
	} else if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
		renderer.move(0, CAMERA_DELTA);
		renderer.moveRects(0, CAMERA_DELTA);
	} else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
		renderer.move(0, -CAMERA_DELTA);
		renderer.moveRects(0, -CAMERA_DELTA);
	}
}

SceneEditor::SceneEditor(EditorRenderer& renderer) {
	renderer.setBackground(&m_background);
}

nv::editor::EditorDest SceneEditor::operator()(EditorRenderer& renderer) {
	const auto mousePos = convertPair<Coord>(ImGui::GetMousePos());

	showBackgroundOptions(renderer);

	selectObject(mousePos); //select sprite or rect

	if (!m_sprites.empty()) {
		if (m_selectedSpriteData != nullptr) {
			m_spriteRectEditor.drag(mousePos);
			editSelectedSprite(renderer);
		}
	}

	editSelectedRect(mousePos);

	if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
		m_showingRightClickOptions = true;
		m_rightClickWinPos = convertPair<ImVec2>(mousePos);
	}
	if (m_showingRightClickOptions) {
		showRightClickOptions(renderer, mousePos);
	}

	moveCamera(renderer);

	return EditorDest::None;
}