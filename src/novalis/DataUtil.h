#ifndef DATA_UTIL_H
#define DATA_UTIL_H

#include <assert.h>
#include <concepts>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <numeric>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include <boost/container/flat_map.hpp>

#include <nlohmann/json.hpp>

#include <SDL2/SDL.h>

#include "Rect.h"

#include "GlobalMacros.h"


void to_json(nlohmann::json& j, const SDL_Color& c);
void from_json(const nlohmann::json& j, SDL_Color& c);

void to_json(nlohmann::json& j, const SDL_Rect& c);
void from_json(const nlohmann::json& j, SDL_Rect& c);

namespace nv {
	using nlohmann::json;
	using namespace std::literals;

	void to_json(json& j, const Rect& r);
	void from_json(const json& j, Rect& r);

	template<std::integral... Nums>
	void parseUnderscoredNums(const std::string& line, Nums&... nums)
	{
		size_t index = 0;

		auto numify = [&](auto& x) {
			size_t iIndex = index; //initial index

			while (true) {
				index++;
				if (line[index] == '_' || index == line.size()) {
					index++;
					break;
				}
			}

			x = std::stoi(line.substr(iIndex, index));
		};

		(numify(nums), ...);
	}

	const std::string& workingDirectory();
	
	inline std::string objectPath(std::string relativePath) {
		return workingDirectory() + std::string("static_objects/") + relativePath;
	}

	inline std::string imagePath(std::string relativePath) {
		return workingDirectory() + std::string("images/") + relativePath;
	}

	//returns the path relative to the working directory
	inline std::string relativePath(std::string relativePath) {
		return workingDirectory() + relativePath;
	}

	template<typename Stream>
	Stream& operator<<(Stream& stream, const SDL_Rect& rect) {
		stream << rect.x << "_" << rect.y << "_" << rect.w << "_" << rect.h << std::endl;
	}

	template<typename Stream, typename... Args>
	void writeSection(Stream& stream, std::string title, Args&&... args) {
		stream << title + " {\n";
		((stream << args << '\n'), ...);
		stream << "}\n";
	}

	std::optional<std::string> fileExtension(const std::string& fileName);

	template<typename... Args>
	void println(Args&&... args) {
		((std::cout << std::forward<Args>(args) << " "), ...);
		std::cout << '\n';
	}

	template<typename T, typename U>
	using FlatOrderedMap = boost::container::flat_map<T, U>;
};

#endif