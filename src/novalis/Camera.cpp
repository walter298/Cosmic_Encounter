#include "Camera.h"

void nv::from_json(const nlohmann::json& j, CameraDelta& camDelta) {
	camDelta.dx = j.at("dx").get<int>();
	camDelta.dy = j.at("dy").get<int>();
	camDelta.time = j.at("time").get<std::chrono::milliseconds>();
}

void nv::to_json(nlohmann::json& j, const CameraDelta& camDelta) {
	j["dx"] = camDelta.dx;
	j["dy"] = camDelta.dy;
	j["time"] = camDelta.time;
}

nv::CameraPath::CameraPath(const std::string& path) {
	std::ifstream file{ path };
	assert(file.is_open());

	auto json = nlohmann::json::parse(file);
	file.close(); //not exception-safe

	m_path = json.get<Path>();
}

const nv::CameraPath::Path& nv::CameraPath::getPath() const noexcept {
	return m_path;
}