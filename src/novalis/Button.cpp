#include "Button.h"

int nv::Button::IDCount = 0;

nv::Button::Button() noexcept {
	IDCount++;
	m_ID = IDCount;
}

nv::Button::Button(Rect* rect) noexcept
	: Button()
{
	m_rect = rect;
}

int nv::Button::getID() const noexcept {
	return m_ID;
}

void nv::Button::onHovered(EventFunc&& func) {
	m_queuedEvents.emplace_back(
		[func = std::move(func), this]() mutable { func(); m_previouslyHovered = true; } ,
		[this]() {
			auto [mx, my] = InputHandler::getInstance().mouse();
			return m_rect->isCoordContained(mx, my);
		} 
	);
}

void nv::Button::onUnhovered(EventFunc&& func) {
	m_queuedEvents.emplace_back(
		[func = std::move(func), this]() mutable { func(); m_previouslyHovered = false; } ,
		[this]() {
			auto [mx, my] = InputHandler::getInstance().mouse();
			return !m_rect->isCoordContained(mx, my) && m_previouslyHovered;
		}
	);
}

void nv::Button::onLeftClick(EventFunc&& func) {
	m_queuedEvents.emplace_back(std::move(func),
		[this]() { 
			auto [mx, my] = InputHandler::getInstance().mouse();
			return InputHandler::getInstance().leftMouseClicked() && m_rect->isCoordContained(mx, my);
		} 
	);
}

void nv::Button::onRightClick(EventFunc&& func) {
	auto [mx, my] = InputHandler::getInstance().mouse();
	m_queuedEvents.emplace_back(func, 
		[this]() { 
			auto [mx, my] = InputHandler::getInstance().mouse();
			return InputHandler::getInstance().rightMouseClicked() && m_rect->isCoordContained(mx, my); 
		}
	);
}

void nv::Button::whileLeftHeld(EventFunc&& func) {
	m_queuedEvents.emplace_back(std::move(func),
		[this]() { 
			auto [mx, my] = InputHandler::getInstance().mouse();
			return InputHandler::getInstance().leftMouseHeld() && m_rect->isCoordContained(mx, my);
		}
	);
}

void nv::Button::operator()() {
	for (auto& evt : m_queuedEvents) {
		evt();
	}
}