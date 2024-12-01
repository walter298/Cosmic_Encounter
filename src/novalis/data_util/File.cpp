#include "File.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <filesystem>
#include <numeric>
#include <ranges>

#include "Algorithms.h"
#include "DataStructures.h"

nv::FileOpenResult nv::openFile(const nv::FileExtensionFilters& filters) {
	NFD::UniquePath outPath;
	
	auto result = NFD::OpenDialog(outPath, filters.begin(), static_cast<nfdfiltersize_t>(filters.size()));
	if (result == NFD_OKAY) {
		return std::string{ outPath.get() };
	} else {
		return std::nullopt;
	}
}

nv::MultipleFileOpensResult nv::openMultipleFiles(const nv::FileExtensionFilters& filters) {
	NFD::UniquePathSet outPaths = nullptr;

	auto res = NFD::OpenDialogMultiple(outPaths, filters.begin(), static_cast<nfdfiltersize_t>(filters.size()));
	if (res != NFD_OKAY) {
		return std::nullopt;
	}

	nfdpathsetsize_t pathC = 0;
	if (NFD::PathSet::Count(outPaths, pathC) != NFD_OKAY) {
		return std::nullopt;
	}

	std::vector<std::string> strPaths;
	strPaths.reserve(pathC);
	for (nfdpathsetsize_t i = 0; i < pathC; i++) {
		NFD::UniquePathSetPath currPath;
		NFD::PathSet::GetPath(outPaths, i, currPath);
		strPaths.push_back(currPath.get());
	}

	return strPaths;
}

bool nv::saveNewFile(const nv::FileExtensionFilters& filters, const nv::FileContentsGenerator& contentsGen) {
	NFD::UniquePath path;
	if (NFD::SaveDialog(path, filters.begin(), static_cast<nfdfiltersize_t>(filters.size())) != NFD_OKAY) {
		return false;
	}

	std::ofstream file{ path.get() };
	file << contentsGen(path.get());
	file.close();

	return true;
}

std::string& nv::convertFullToRegularPath(std::string& path) {
	auto relativePathSize = relativePath("").size();
	path.erase(path.begin(), path.begin() + relativePathSize);
	return path;
}

std::string nv::convertFullToRegularPath(std::string_view path) {
	std::string pathStr = path.data();
	convertFullToRegularPath(pathStr);
	return pathStr;
}

const std::string& nv::workingDirectory() { //should be called by nv::Instance constructor
	static auto path = [] {
		auto path = std::filesystem::current_path().string() + "/";
		std::ranges::replace(path, '\\', '/');
		return path;
	}();
	return path;
}

/*Have thread local string to prevent dangling pointers when relativePath is assigned to string_view*/
const std::string& nv::relativePath(std::string_view relativePath) {
	thread_local std::string global;
	global = workingDirectory() + relativePath.data();
	return global;
}

std::optional<std::string> nv::fileExtension(const std::string& fileName) {
	using namespace std::literals;

	auto dotPos = std::ranges::find(fileName, '.');
	if (dotPos == fileName.end()) {
		return std::nullopt;
	}
	return std::accumulate(dotPos, fileName.end(), ""s);
}

std::string_view nv::fileName(std::string_view filePath) {
	static std::string nonDanglingFilename;
	nonDanglingFilename = filePath.data();

	auto slashIdx = nonDanglingFilename.find_last_of('\\');
	if (slashIdx == std::string::npos) {
		slashIdx = nonDanglingFilename.find_last_of('/');
		assert(slashIdx != std::string::npos);
	}
	auto dotIdx = nonDanglingFilename.find_last_of('.');
	assert(dotIdx != std::string::npos);
	nonDanglingFilename = filePath.substr(slashIdx + 1, dotIdx - slashIdx - 1);

	return nonDanglingFilename;
}
