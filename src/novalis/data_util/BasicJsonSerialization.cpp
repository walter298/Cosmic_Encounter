#include "BasicJsonSerialization.h"

#include <fstream>
#include <print>

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

nlohmann::json nv::parseJson(std::string_view filename) {
	std::ifstream file{ filename.data() };
	assert(file.is_open());
	return json::parse(file);
}

void nv::printJsonKeys(const json& json) {
	for (const auto& key : json.items()) {
		std::println("{}", key.key().c_str());
	}
}
