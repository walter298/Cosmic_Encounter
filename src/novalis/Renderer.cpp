#include "Renderer.h"

nv::Renderer::Renderer(SDL_Renderer* renderer) 
	: m_renderer(renderer)
{
}

void nv::Renderer::move(int dx, int dy) noexcept {
	m_background->ren.move(dx, dy);
	for (auto& [layer, sprites] : m_objects) {
		for (auto& sprite : sprites) {
			sprite->ren.move(dx, dy);
		}
	}
}

void nv::Renderer::clear() noexcept {
	m_objects.clear();
}

void nv::Renderer::setBackground(Background* background) noexcept {
	m_background = background;
}

void nv::Renderer::addObj(Sprite* sprite, int layer) {
	m_objects[layer].insert(sprite);
}

void nv::Renderer::removeObj(const ID& ID, int layer) {
	auto objPos = std::ranges::find_if(m_objects.at(layer),
		[&ID](const Sprite* obj) { return obj->getID() == ID; }
	);
	assert(objPos != m_objects.at(layer).end());
	m_objects.at(layer).erase(objPos);
}

void nv::Renderer::render() noexcept {
	SDL_RenderClear(m_renderer);
	m_background->render(m_renderer);
	for (auto& [layer, objs] : m_objects) {
		for (auto& obj : objs) {
			obj->render(m_renderer);
		}
	}
	SDL_RenderPresent(m_renderer);
}
