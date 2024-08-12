#include "Camera.h"

nv::CameraPath::CameraPath(const std::string& path) {
	std::ifstream file{ path };
	assert(file.is_open());

	auto json = nlohmann::json::parse(file);
	file.close(); //not exception-safe

	//m_path = json.get<Path>();
}

const nv::CameraPath::Path& nv::CameraPath::getPath() const noexcept {
	return m_path;
}