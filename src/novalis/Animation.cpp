#include "Animation.h"

#include <cassert>

nv::Animation::Animation(int firstTexIdx, int lastTexIdx) noexcept
	: m_firstTexIdx{ firstTexIdx }, m_lastTexIdx{ lastTexIdx }
{
	assert(m_lastTexIdx > m_firstTexIdx);
	m_currTexIdx = m_firstTexIdx;
}

int nv::Animation::operator()() {
	if (!m_startedAnimating) {
		m_currTexIdx = m_firstTexIdx;
		m_startedAnimating = true;
		return m_currTexIdx;
	}

	m_currTexIdx = m_currTexIdx == m_lastTexIdx ? m_firstTexIdx : m_currTexIdx + 1;

	return m_currTexIdx;
}

void nv::Animation::cancel() noexcept {
	m_startedAnimating = false;
}