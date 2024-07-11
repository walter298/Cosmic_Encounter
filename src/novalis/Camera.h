#pragma once

#include <cassert>
#include <chrono>
#include <fstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "DataUtil.h"

namespace nv {
	struct CameraDelta {
		int dx = 0;
		int dy = 0;
		using DeltaTimeT = std::chrono::milliseconds;
		DeltaTimeT time{ 0 };
	};

	void from_json(const nlohmann::json& j, CameraDelta& camDelta);
	void to_json(nlohmann::json& j, const CameraDelta& camDelta);

	class CameraPath {
	private:
		using Path = std::vector<CameraDelta>;
		Path m_path;
	public:
		CameraPath(const std::string& path);
		const Path& getPath() const noexcept;
	};

	
}