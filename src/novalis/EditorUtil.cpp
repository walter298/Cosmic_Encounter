#include "EditorUtil.h"

using nv::editor::ImGuiInstance;

ImGuiIO& ImGuiInstance::initIO() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	return ImGui::GetIO();
}

ImGuiInstance::ImGuiInstance(SDL_Window* window, SDL_Renderer* renderer) : m_io(initIO()) {
	(void)m_io;
	m_io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
	ImGui_ImplSDLRenderer2_Init(renderer);
}

ImGuiInstance::~ImGuiInstance() {
	ImGui_ImplSDLRenderer2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

ImGuiIO& ImGuiInstance::getIO() {
	return m_io;
}

using nv::editor::Editor;

Editor::Editor(ImGuiIO& io, SDL_Renderer* renderer)
	: m_io{ io }, m_renderer{ renderer } {}

void Editor::execute() {
	bool running = true;

	InputHandler::getInstance().useImGui();

	Event quitEvt{
		[&running] { running = false; },
		[] { return InputHandler::getInstance().eventStates(SDL_QUIT); }
	};

	while (running) {
		auto waitTime = 1000ms / NV_FPS;
		auto endTime = system_clock::now() + waitTime;

		InputHandler::getInstance().run();

		quitEvt();

		ImGui_ImplSDLRenderer2_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		showGui(running);

		const auto now = system_clock::now();

		//checks frames, render
		if (now < endTime) {
			std::this_thread::sleep_for(endTime - now);
		}
		ImVec4 color{ 0.45f, 0.55f, 0.60f, 1.00f };

		ImGui::Render();
		SDL_RenderSetScale(m_renderer.get(), m_io.DisplayFramebufferScale.x, m_io.DisplayFramebufferScale.y);
		SDL_SetRenderDrawColor(m_renderer.get(),
			//unfortunately SDL uses ints for screen pixels and ImGui uses floats 
			static_cast<Uint8>(color.x * 255), static_cast<Uint8>(color.y * 255),
			static_cast<Uint8>(color.z * 255), static_cast<Uint8>(color.w * 255));
		SDL_RenderClear(m_renderer.get());
		ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());

		customRender();

		SDL_RenderPresent(m_renderer.get());
	}
}

void nv::editor::editRect(Rect& rect, bool editingColor) {
	static std::array<int, 2> size = { 0, 0 };

	if (ImGui::SliderInt2("Size", size.data(), 0, 1000)) {
		rect.setSize(size[0], size[1]);
	}

	static std::array<float, 4> rgba = { 0, 0, 0, 0 };

	if (editingColor) {
		if (ImGui::ColorEdit4("Color", rgba.data())) {
			rect.setRenderColor(
				static_cast<Uint8>(rgba[0] * 255),
				static_cast<Uint8>(rgba[1] * 255),
				static_cast<Uint8>(rgba[2] * 255),
				static_cast<Uint8>(rgba[3] * 255)
			);
		}
	}
}

std::optional<std::string> nv::editor::openFilePath() {
	WCHAR buffer[MAX_PATH];
	OPENFILENAME ofn = {};
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFilter = TEXT("texture files\0*.txt\0");
	ofn.lpstrFile = buffer, ofn.nMaxFile = MAX_PATH, * buffer = '\0';
	ofn.Flags = OFN_EXPLORER | OFN_ENABLESIZING | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn)) {
		std::wstring wpath = ofn.lpstrFile; //store path in wstring so we can convert it to a normal std::string
		std::ranges::replace(wpath, L'\\', L'/');
		return std::string{ wpath.begin(), wpath.end() };
	} else {
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
		wchar_t* filePart = ofn.lpstrFile + wdirectory.size() + 1;
		if (*filePart == L'\0') {
			ret.push_back({ wdirectory.begin(), wdirectory.end() });
		} else {
			while (*filePart != L'\0') {
				std::wstring wpath = wdirectory + L"\\" + filePart;
				ret.push_back({ wpath.begin(), wpath.end() });
				filePart += wcslen(filePart) + 1;
			}
		}
		return ret;
	} else {
		return std::nullopt;
	}
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

using nv::editor::DragCheck;

void DragCheck::add(nv::Rect& rect) 
{
	m_rects.push_back(std::ref(rect));
}

void DragCheck::drag() {
	auto [mx, my] = InputHandler::getInstance().mouse();
	auto [dx, dy] = InputHandler::getInstance().mouseChange();

	bool isHovered = false;
	for (const auto& rect : m_rects) {
		if (rect.get().isCoordContained(mx, my)) {
			isHovered = true;
			break;
		}
	}
	if (isHovered && InputHandler::getInstance().leftMouseHeld()) {
		for (auto& rect : m_rects) {
			rect.get().move(dx, dy);
		}
	}
}