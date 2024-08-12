#include "File.h"

#include <algorithm>
#include <filesystem>
#include <numeric>

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
	auto slashIdx = filePath.find_last_of('\\');
	auto dotIdx = filePath.find_last_of('.');
	return filePath.substr(slashIdx + 1, dotIdx - slashIdx - 1);
}
