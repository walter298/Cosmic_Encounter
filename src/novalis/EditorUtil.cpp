#include "EditorUtil.h"

std::optional<std::string> nv::editor::openFilePath() {
	WCHAR buffer[MAX_PATH];
	OPENFILENAME ofn = {};
	ofn.lStructSize = sizeof(ofn);
	//ofn.lpstrFilter = TEXT("texture files\0*.txt\0");
	ofn.lpstrFile = buffer, ofn.nMaxFile = MAX_PATH, * buffer = '\0';
	ofn.Flags = OFN_EXPLORER | OFN_ENABLESIZING | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn)) {
		std::wstring wpath = ofn.lpstrFile; //store path in wstring so we can convert it to a normal std::string
		ranges::replace(wpath, L'\\', L'/');
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

	if (GetSaveFileName(&ofn)) {
		HANDLE hFile = CreateFile(ofn.lpstrFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE) {
			DWORD dwBytesWritten;
			WriteFile(hFile, "", strlen(""), &dwBytesWritten, NULL);
			CloseHandle(hFile);
			std::wstring wpath = szFile;
			std::ranges::replace(wpath, L'\\', L'/');
			return std::string{ wpath.begin(), wpath.end() };
		}
	}
	return std::nullopt;
}