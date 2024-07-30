#include "DataUtil.h"

#include <fstream>

template<std::integral T>
using Quad = std::tuple<T, T, T, T>;

void to_json(nlohmann::json& j, const SDL_Color& c) {
	j = std::tie(c.r, c.g, c.b, c.a);
}

void from_json(const nlohmann::json& j, SDL_Color& c) {
	std::tie(c.r, c.g, c.b, c.a) = j.get<Quad<Uint8>>();	
}

void to_json(nlohmann::json& j, const SDL_Rect& r) {
	j = std::tie(r.x, r.y, r.w, r.h);
}

void from_json(const nlohmann::json& j, SDL_Rect& r) {
	std::tie(r.x, r.y, r.w, r.h) = j.get<Quad<int>>();
}

void to_json(nlohmann::json& j, const SDL_Point& p) {
	j = std::tie(p.x, p.y);
}
void from_json(const nlohmann::json& j, SDL_Point& p) {
	std::tie(p.x, p.y) = j.get<std::tuple<int, int>>();
}

const std::string& nv::workingDirectory() {
	static auto path = [] {
		auto path = std::filesystem::current_path().string() + "/";
		std::ranges::replace(path, '\\', '/');
		return path;
	}();
	return path;
}

/*Have thread local string to prevent dangling pointers when relativePath is assigned to string_view*/
std::string nv::relativePath(std::string_view relativePath) {
	thread_local std::string global;
	global = workingDirectory() + relativePath.data();
	return global;
}

std::optional<std::string> nv::fileExtension(const std::string& fileName) {
	auto dotPos = ranges::find(fileName, '.');
	if (dotPos == fileName.end()) {
		return std::nullopt;
	}
	return std::accumulate(dotPos, fileName.end(), ""s);
}

std::string_view nv::fileName(std::string_view filePath) {
	auto slashIdx = filePath.find_last_of('\\');
	auto dotIdx   = filePath.find_last_of('.');
	return filePath.substr(slashIdx + 1, dotIdx - slashIdx - 1);
}

std::string nv::writeCloneID(std::string_view str) {
	return "clone_"s + str.data();
}

nlohmann::json nv::parseFile(std::string_view filename) {
	std::ifstream file{ filename.data() };
	assert(file.is_open());
	return json::parse(file);
}

nlohmann::json nv::parseJson(const std::string& path) {
	std::ifstream file{ path };
	assert(file.is_open());
	return json::parse(file);
}

const std::string& nv::NamedObject::getName() const noexcept {
	return m_name;
}
