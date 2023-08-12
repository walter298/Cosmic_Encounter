#include "EditorUtil.h"

using nv::editor::ImGuiInstance;

ImGuiIO& ImGuiInstance::initIO() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	return ImGui::GetIO();
}

ImGuiInstance::ImGuiInstance(SDL_Window* window, Renderer& renderer) : m_io(initIO()) {
	(void)m_io;
	m_io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForSDLRenderer(window, renderer.get());
	ImGui_ImplSDLRenderer2_Init(renderer.get());
}

ImGuiInstance::~ImGuiInstance() {
	ImGui_ImplSDLRenderer2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

ImGuiIO& ImGuiInstance::getIO() {
	return m_io;
}

void nv::editor::runEditor(ImGuiIO& io, Renderer& renderer, std::function<void(bool&)> showGui) {
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
		renderer.renderImgui(io);
	}
}

void nv::editor::editRect(Rect& rect) {
	static std::array<float, 4> rgba = { 0, 0, 0, 0 };

	if (ImGui::ColorEdit4("Color", rgba.data())) {
		rect.setRenderColor(
			static_cast<Uint8>(rgba[0] * 255),
			static_cast<Uint8>(rgba[1] * 255),
			static_cast<Uint8>(rgba[2] * 255),
			static_cast<Uint8>(rgba[3] * 255)
		);
	}

	static std::array<int, 2> size = { 0, 0 };

	if (ImGui::SliderInt2("Size", size.data(), 0, 1000)) {
		rect.setSize(size[0], size[1]);
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
		std::string path(wpath.begin(), wpath.end()); //convert wstring to std::string
		std::replace(path.begin(), path.end(), '\\', '/'); //replace the windows backslashes with normal backslashes
		return path;
	} else {
		return std::nullopt;
	}
}

std::optional<std::string> nv::editor::saveFile(std::wstring openMessage) {
	HWND hwnd = GetActiveWindow();
	OPENFILENAME ofn;
	WCHAR szFile[MAX_PATH] = { 0 };
	WCHAR* szInitialFileName = openMessage.data(); // Default initial file name

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
			std::string path{ wpath.begin(), wpath.end() }; //convert wstring to normal std::string
			return path;
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