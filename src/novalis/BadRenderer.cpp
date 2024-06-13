#include "BadRenderer.h"

#include "BadRenderer.h"

#include <print>

void nv::BadRenderer::renderCopyObjs() {
	auto render = [this](auto& layersOfObjs) {
		for (auto& [layer, objs] : layersOfObjs) {
			for (auto& obj : objs) {
				obj->render(m_renderer);
			}
		}
		};
	render(m_sprites);
	render(m_rects);
	render(m_textures);
}

nv::BadRenderer::BadRenderer(SDL_Renderer* renderer)
	: m_renderer(renderer)
{
}

void nv::BadRenderer::move(int dx, int dy) noexcept {
	auto moveImpl = [&](auto objLayers) {
		for (auto& [layer, objs] : objLayers) {
			for (auto& sprite : objs) {
				sprite->move(dx, dy);
			}
		}
		};

	//move(m_rects);
	moveImpl(m_sprites);
}

void nv::BadRenderer::clear() noexcept {
	m_sprites.clear();
	m_textures.clear();
	m_rects.clear();
}

void nv::BadRenderer::add(Sprite* sprite, int layer) {
	m_sprites[layer].insert(sprite);
}
void nv::BadRenderer::erase(const Sprite* const sprite, int layer) {
	removeObjImpl(m_sprites[layer], sprite);
}

void nv::BadRenderer::add(Rect* rect, int layer) {
	m_rects[layer].insert(rect);
}

void nv::BadRenderer::removeRect(Rect* rect, int layer) {
	removeObjImpl(m_rects[layer], rect);
}

void nv::BadRenderer::add(TextureData* texData, int layer) {
	m_textures[layer].insert(texData);
}

void nv::BadRenderer::erase(const TextureData* const texData, int layer) {
	removeObjImpl(m_textures[layer], texData);
}

void nv::BadRenderer::render() noexcept {
	SDL_RenderClear(m_renderer);
	renderCopyObjs();
	SDL_ShowCursor(showingCursor);
	SDL_RenderPresent(m_renderer);
}

void nv::BadRenderer::renderWithImGui(ImGuiIO& io) {
	static constexpr ImVec4 color{ 0.45f, 0.55f, 0.60f, 1.00f };
	ImGui::Render();
	SDL_RenderSetScale(m_renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
	SDL_SetRenderDrawColor(m_renderer,
		//unfortunately SDL uses ints for screen pixels and ImGui uses floats 
		static_cast<Uint8>(color.x * 255), static_cast<Uint8>(color.y * 255),
		static_cast<Uint8>(color.z * 255), static_cast<Uint8>(color.w * 255));
	SDL_RenderClear(m_renderer);
	ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
	renderCopyObjs();
	SDL_ShowCursor(showingCursor);
	SDL_RenderPresent(m_renderer);
}