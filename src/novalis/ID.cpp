#include "ID.h"

nv::ID::ID() noexcept : m_ID{ std::chrono::steady_clock::now() } {}
nv::ID::ID(const ID& other) noexcept : m_ID{ std::chrono::steady_clock::now() } {}
nv::ID::ID(ID&& other) noexcept : m_ID{ std::move(other.m_ID) } {}

nv::ID& nv::ID::operator=(const ID& other) noexcept {
	m_ID = std::chrono::steady_clock::now();
	return *this;
}
nv::ID& nv::ID::operator=(ID&& other) noexcept {
	m_ID = std::move(other.m_ID);
	return *this;
}
bool nv::ID::operator==(const ID& other) const noexcept {
	return (m_ID == other.m_ID);
}