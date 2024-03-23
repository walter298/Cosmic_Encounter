#pragma once

#include <chrono>

namespace nv {
	class ID {
	private:
		std::chrono::steady_clock::time_point m_ID;
	public:
		ID() noexcept;
		ID(const ID& other) noexcept;
		ID(ID&& other) noexcept;
		ID& operator=(const ID& other) noexcept;
		ID& operator=(ID&& other) noexcept;
		bool operator==(const ID& other) const noexcept;
	};
}