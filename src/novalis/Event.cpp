#include "Event.h"

bool nv::EventChain::operator()() {
	if (m_currEvent == nullptr) {
		m_currEvent = &m_events.front();
	}
	if ((*m_currEvent)()) {
		if (m_currEvent == &m_events.back()) {
			return true;
		} else {
			m_currEvent++;
			return false;
		}
	}
	return false;
}