#ifndef DATA_UTIL_H
#define DATA_UTIL_H

#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <numeric>
#include <regex>
#include <string>
#include <type_traits>
#include <vector>

#include <SDL.h>

#include "GlobalMacros.h"

namespace nv {
	template<typename Map, typename KeyType>
	void checkKeyValidity(const Map& map, const KeyType& invalidKey) {
		if (!map.contains(invalidKey)) {
			using namespace std::literals;
			throw std::runtime_error("Error: invalid key accessed for "s + typeid(map).name());
		}
	}

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

	template<std::integral... Nums> 
	std::string writeNums(Nums... nums) {
		std::string ret;

		int argCount = sizeof...(nums);

		auto add = [&](auto num) {
			argCount--;
			if (argCount == 0) {
				ret.append(std::to_string(num));
			} else {
				ret.append(std::to_string(num) + "_");
			}
		};

		((add(nums)), ...);

		return ret;
	}

	const std::string& workingDirectory();

	template<typename FileStream>
	void checkFileValidity(FileStream& file, std::string path) {
		if (!file.is_open()) {
			throw std::runtime_error("Error: could not open " + path);
		}
	}

	class FileData {
	private:
		class DataSection;
		using DataSectionPtr           = std::unique_ptr<DataSection>;
		using OptionalDataSection      = std::optional<DataSection*>;
		using OptionalMultiDataSection = std::optional<std::vector<DataSection*>>;
		using DataMap                  = std::multimap<std::string, DataSectionPtr>;

		class DataSection {
		private:
			DataMap m_nestedData;
			std::vector<std::string> m_data;
		public:
			DataSection(std::vector<std::string> data);

			std::vector<std::string>& data() noexcept;
			std::string& operator[](size_t idx) noexcept;

			OptionalDataSection getNestedData(std::string title) noexcept;
			OptionalMultiDataSection getNestedDataOccurrences(std::string title) noexcept;
		};

		using It = std::vector<std::string>::iterator;
		static It matchingClosingBrace(It begin, It end); //assumes there are matching braces

		DataMap m_data;

		static OptionalMultiDataSection
			getDataOccurences(const DataMap& mMap, std::string title) noexcept;
	public:
		FileData(std::string absPath);

		OptionalMultiDataSection getMultipleDataSections(std::string string);
		OptionalDataSection      getDataSection(std::string);
	};

	std::tuple<std::string, int, int> staticObjectData(std::string& line);

	inline std::string objectPath(std::string relativePath) {
		return workingDirectory() + std::string("static_objects/") + relativePath;
	}

	inline std::string imagePath(std::string relativePath) {
		return workingDirectory() + std::string("images/") + relativePath;
	}

	//returns the path relative to the working directory
	inline std::string relativePath(std::string relativePath) {
		return workingDirectory() + std::string(relativePath);
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
		std::cout << std::endl;
	}
}

#endif