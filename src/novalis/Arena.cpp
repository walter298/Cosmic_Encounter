#include "Arena.h"

nv::Arena::Arena(size_t size) noexcept
	: m_arena{ std::make_unique<std::byte[]>(size) }, m_capacity{ size }
{
}

void* nv::Arena::alloc(size_t byteC) noexcept {
	assert(m_used + byteC < m_capacity);
	m_used += byteC;
	return nullptr;
}

