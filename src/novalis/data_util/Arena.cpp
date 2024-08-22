#include "Arena.h"

nv::Arena::Arena(size_t size)
	: m_buff{ std::make_unique<std::byte[]>(size) }, m_nextObjectBegin{ m_buff.get() }, m_space { size }
{
}

void* nv::Arena::rawAlloc(size_t bytes) noexcept {
	if (std::align(bytes, sizeof(std::max_align_t), m_nextObjectBegin, m_space)) {
		auto temp = m_nextObjectBegin;
		m_nextObjectBegin = static_cast<std::byte*>(m_nextObjectBegin) + bytes;
		return temp;
	} else {
		return nullptr;
	}
}

void nv::Arena::clear() noexcept {
	m_space = 0;
	m_nextObjectBegin = m_buff.get();
}
