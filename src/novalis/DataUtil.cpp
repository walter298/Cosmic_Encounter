#include "DataUtil.h"

nv::FileData::DataSection::DataSection(std::vector<std::string> data) 
	: m_data(data.begin(), data.end() - 1) 
{
	for (auto it = data.begin(); it != data.end(); it++) {
		if (it->empty()) {
			continue;
		}
		if (it->contains('{')) {
			auto closingBracePos = matchingClosingBrace(it, data.end());
			m_nestedData.insert(std::make_pair(*it, std::make_unique<DataSection>(
				std::vector<std::string>{ std::next(it), std::next(closingBracePos) }
			)));
			if (std::next(closingBracePos) != data.end()) {
				it = std::next(closingBracePos);
			} else {
				break;
			}
		}
	}
}

std::vector<std::string>& nv::FileData::DataSection::data() noexcept 
{
	return m_data;
}

std::string& nv::FileData::DataSection::operator[](size_t idx) noexcept {
	return m_data[idx];
}

nv::FileData::OptionalDataSection nv::FileData::DataSection::getNestedData(std::string title) noexcept
{
	if (!m_nestedData.contains(title)) {
		return std::nullopt;
	} else {
		return m_nestedData.find(title)->second.get();
	}
}

std::optional<std::vector<nv::FileData::DataSection*>>
	nv::FileData::DataSection::getNestedDataOccurrences(std::string title) noexcept 
{
	return getDataOccurences(m_nestedData, title);
}

nv::FileData::OptionalMultiDataSection
	nv::FileData::getDataOccurences(const nv::FileData::DataMap& mMap, std::string title) noexcept
{
	if (!mMap.contains(title)) {
		return std::nullopt;
	}
	std::vector<DataSection*> ret;
	auto data = mMap.equal_range(title);
	for (auto it = data.first; it != data.second; it++) {
		ret.push_back(it->second.get());
	}

	return ret;
}

nv::FileData::It nv::FileData::matchingClosingBrace(It begin, It end) {
	int openingBraceCount = 0;
	int closingBraceCount = 0;
	
	It currentPos = begin;

	while (true) {
		auto nextBrace = std::find_if(currentPos, end,
			[](const auto& str) { return str.contains('{') || str.contains('}'); }
		);
		if (nextBrace == end) {
			return end;
		}
		if (nextBrace->contains('}')) {
			closingBraceCount++;
		} else {
			openingBraceCount++;
		}
		if (openingBraceCount == closingBraceCount) {
			return nextBrace;
		} else {
			currentPos = std::next(nextBrace);
		}
	}
}

nv::FileData::FileData(std::string absPath) {
	std::ifstream file;
	file.open(absPath);
	checkFileValidity(file, absPath);

	std::vector<std::string> fileContents;
	std::string line;

	while (std::getline(file, line)) {
		fileContents.push_back(line);
	}

	auto it = fileContents.begin();
	while (it != fileContents.end()) {
		if (it->empty()) {
			continue;
		}

		auto closingBracePos = matchingClosingBrace(it, fileContents.end());
		if (closingBracePos == fileContents.end()) {
			throw std::runtime_error("Error: no matching closing brace found in " + absPath);
		}
		m_data.insert(
			std::make_pair(
				*it,
				std::make_unique<DataSection>(std::vector<std::string>{ std::next(it), std::next(closingBracePos) })
			)
		);
		it = std::next(closingBracePos);
	}
}

nv::FileData::OptionalDataSection nv::FileData::getDataSection(std::string title) {
	if (m_data.contains(title)) {
		return m_data.find(title)->second.get();
	} else {
		return std::nullopt;
	}
}

nv::FileData::OptionalMultiDataSection nv::FileData::getMultipleDataSections(std::string title) {
	if (m_data.contains(title)) {
		return getDataOccurences(m_data, title);
	} else {
		return std::nullopt;
	}
}

std::tuple<std::string, int, int> nv::staticObjectData(std::string& line) {
	std::string path;
	int x, y;

	size_t numIndex = 0; //index of numbers in line 

	//parse path
	for (numIndex = 0; numIndex < line.size(); numIndex++) {
		if (line[numIndex] == '@') {
			numIndex++;
			break;
		}
		path.push_back(line[numIndex]);
	}

	//parse numbers
	parseUnderscoredNums(line.substr(numIndex, line.size() - 1), x, y);

	return std::make_tuple(path, x, y);
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
	using namespace std::literals;
	return std::accumulate(dotPos, fileName.end(), ""s);
}