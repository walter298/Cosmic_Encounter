#pragma once

#include <cassert>
#include <cstddef>
#include <memory>

namespace nv {
	class Arena {
	private:
		const std::unique_ptr<std::byte[]> m_arena;
		const size_t m_capacity; //in bytes
		size_t m_used = 0;
	public:
		Arena(size_t size) noexcept;

		void* alloc(size_t byteC) noexcept;
	};
}