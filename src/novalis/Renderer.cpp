#include "Renderer.h"

nv::Renderer::Renderer(SDL_Renderer* renderer) 
	: m_renderer(renderer)
{
}

void nv::Renderer::clear() noexcept {
	m_objects.clear();
}

void nv::Renderer::addObj(RenderObj* obj) noexcept {
	auto objPos = std::ranges::upper_bound(m_objects, obj,
		[](const auto& a, const auto& b) { return a->getLayer() < b->getLayer(); }
	);
	m_objects.insert(objPos, obj);
}

void nv::Renderer::removeObj(int ID) {
	std::erase_if(m_objects, [&ID](const auto& obj) { return obj->getID() == ID; });
}

void nv::Renderer::render() noexcept {
	SDL_RenderClear(m_renderer);

	for (const auto& obj : m_objects) {
		obj->render(m_renderer);
	}

	SDL_RenderPresent(m_renderer);
}

void nv::Renderer::renderImgui(ImGuiIO& io) {
	ImVec4 color{ 0.45f, 0.55f, 0.60f, 1.00f };

	ImGui::Render();
	SDL_RenderSetScale(m_renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
	SDL_SetRenderDrawColor(m_renderer,
		//unfortunately SDL uses ints for screen pixels and ImGui uses floats 
		static_cast<Uint8>(color.x * 255), static_cast<Uint8>(color.y * 255),
		static_cast<Uint8>(color.z * 255), static_cast<Uint8>(color.w * 255));
	SDL_RenderClear(m_renderer);
	ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());

	for (const auto& obj : m_objects) {
		obj->render(m_renderer);
	}

	SDL_RenderPresent(m_renderer);
}