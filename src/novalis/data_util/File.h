#pragma once

#include <optional>
#include <string>

namespace nv {
	const std::string& workingDirectory();

	//returns the path relative to the working directory
	const std::string& relativePath(std::string_view relativePath);

	std::optional<std::string> fileExtension(const std::string& fileName);
	std::string_view fileName(std::string_view filePath);

	std::string& convertFullToRegularPath(std::string& path);
	std::string convertFullToRegularPath(std::string_view path);
}