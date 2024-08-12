#pragma once

#include <cassert>
#include <chrono>
#include <fstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace nv {
	struct CameraDelta {
		int dx = 0;
		int dy = 0;
		using DeltaTimeT = std::chrono::milliseconds;
		DeltaTimeT time{ 0 };
	};

	class CameraPath {
	private:
		using Path = std::vector<CameraDelta>;
		Path m_path;
	public:
		CameraPath(const std::string& path);
		const Path& getPath() const noexcept;
	};
}