#include "EditorUtil.h"

using nv::editor::EditorRenderer;

void EditorRenderer::resetBackground() noexcept {
	m_background = nullptr;
}

void EditorRenderer::addRect(Rect* rect) {
	m_rects.insert(rect);
}

void EditorRenderer::render(ImGuiIO& io) noexcept {
	ImVec4 color{ 0.45f, 0.55f, 0.60f, 1.00f };
	ImGui::Render();
	SDL_RenderSetScale(m_renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
	SDL_SetRenderDrawColor(m_renderer,
		//unfortunately SDL uses ints for screen pixels and ImGui uses floats 
		static_cast<Uint8>(color.x * 255), static_cast<Uint8>(color.y * 255),
		static_cast<Uint8>(color.z * 255), static_cast<Uint8>(color.w * 255));
	SDL_RenderClear(m_renderer);

	if (m_background != nullptr) {
		m_background->render(m_renderer);
	}
	ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
	for (const auto& [layer, sprites] : m_objects) {
		for (const auto& obj : sprites) {
			obj->render(m_renderer);
		}
	}
	for (const auto& rect : m_rects) {
		rect->render(m_renderer);
	}
	
	SDL_RenderPresent(m_renderer);
}

void EditorRenderer::moveRects(int dx, int dy) noexcept {
	for (auto& rect : m_rects) {
		rect->move(dx, dy);
	}
}

std::optional<std::string> nv::editor::openFilePath() {
	WCHAR buffer[MAX_PATH];
	OPENFILENAME ofn = {};
	ofn.lStructSize = sizeof(ofn);
	//ofn.lpstrFilter = TEXT("texture files\0*.txt\0");
	ofn.lpstrFile = buffer, ofn.nMaxFile = MAX_PATH, * buffer = '\0';
	ofn.Flags = OFN_EXPLORER | OFN_ENABLESIZING | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn)) {
		std::wstring wpath = ofn.lpstrFile; //store path in wstring so we can convert it to a normal std::string
		std::ranges::replace(wpath, L'\\', L'/');
		return std::string{ wpath.begin(), wpath.end() };
	}
	else {
		return std::nullopt;
	}
}

std::optional<std::vector<std::string>> nv::editor::openFilePaths() {
	std::vector<std::string> ret;

	OPENFILENAME ofn;
	wchar_t szFile[MAX_PATH * 100]; // Buffer for file names

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = L"All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_EXPLORER | OFN_ENABLESIZING | OFN_HIDEREADONLY |
		OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT;

	if (GetOpenFileName(&ofn)) {
		std::wstring wdirectory = ofn.lpstrFile;

		//get each file path
		wchar_t* filePart = reinterpret_cast<wchar_t*>(ofn.lpstrFile + wdirectory.size() + 1); //EVIL
		if (*filePart == L'\0') {
			ret.push_back({ wdirectory.begin(), wdirectory.end() });
		}
		else {
			while (*filePart != L'\0') {
				std::wstring wpath = wdirectory + L"\\" + filePart;
				ret.push_back({ wpath.begin(), wpath.end() });
				filePart += wcslen(filePart) + 1;
			}
		}
		return ret;
	}
	return std::nullopt;
}

std::optional<std::string> nv::editor::saveFile(std::wstring openMessage) {
	HWND hwnd = GetActiveWindow();
	OPENFILENAME ofn;
	WCHAR szFile[MAX_PATH] = { 0 };
	WCHAR* szInitialFileName = openMessage.data();

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
	ofn.lpstrDefExt = L"txt";
	ofn.lpstrFileTitle = szInitialFileName; // Initial file name

	// Display the Save dialog
	if (GetSaveFileName(&ofn) == TRUE) {
		// Save the file
		HANDLE hFile = CreateFile(ofn.lpstrFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE) {
			// Replace this with the data you want to save in the file
			const char* dataToSave = "This is an example content to save in the file.";
			DWORD dwBytesWritten;
			WriteFile(hFile, dataToSave, strlen(dataToSave), &dwBytesWritten, NULL);
			CloseHandle(hFile);
			std::wstring wpath = szFile;
			std::ranges::replace(wpath, L'\\', L'/');
			return std::string{ wpath.begin(), wpath.end() };
		}
	}
	return std::nullopt;
}

void nv::editor::loadImages(std::vector<std::string>& imagePaths, TexturePtrs& textures, Renderer& renderer) {
	auto spritePaths = openFilePaths();
	if (spritePaths) {
		auto spritesToAddC = spritePaths->size();

		if (textures.capacity() < spritesToAddC) {
			textures.reserve(spritesToAddC);
		}
		
		for (const auto& path : *spritePaths) {
			textures.push_back(
				std::make_shared<Texture>(IMG_LoadTexture(renderer.get(), path.c_str()))
			);
			imagePaths.push_back(path);
		}
	}
}

void nv::editor::RectEditor::drag(const Coord& mousePos) {
	auto [dmx, dmy] = ImGui::GetMouseDragDelta();
	if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) &&
		rect->isCoordContained(
			static_cast<int>(mousePos.x),
			static_cast<int>(mousePos.y)))
	{ //drag selected texture
		m_dragging = true;
	}
	else if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
		m_dragging = false;
	}
	if (m_dragging) {
		rect->move(static_cast<int>(dmx), static_cast<int>(dmy));
		ImGui::ResetMouseDragDelta();
	}
}

void nv::editor::RectEditor::edit(bool showingColor) {
	if (rect == nullptr) {
		return;
	}
	ImGui::SliderInt("Width", &rect->rect.w, 0, NV_SCREEN_WIDTH);
	ImGui::SliderInt("Height", &rect->rect.h, 0, NV_SCREEN_HEIGHT);

	if (showingColor && ImGui::ColorEdit4("Color", m_floatColors.data())) {
		rect->setRenderColor(
			static_cast<Uint8>(m_floatColors[0] * 255),
			static_cast<Uint8>(m_floatColors[1] * 255),
			static_cast<Uint8>(m_floatColors[2] * 255),
			static_cast<Uint8>(m_floatColors[3] * 255)
		);
	}
}