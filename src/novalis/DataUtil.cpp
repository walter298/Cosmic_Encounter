#include "DataUtil.h"

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

std::optional<std::string> nv::fileExtension(const std::string& fileName) {
	auto dotPos = std::ranges::find(fileName, '.');
	if (dotPos == fileName.end()) {
		return std::nullopt;
	}
	return std::accumulate(dotPos, fileName.end(), ""s);
}

std::string_view nv::fileName(std::string_view filePath) {
	auto dotIdx = filePath.find_last_of('.');
	assert(dotIdx != std::string::npos && dotIdx + 1 < filePath.size());
	return filePath.substr(dotIdx + 1);
}