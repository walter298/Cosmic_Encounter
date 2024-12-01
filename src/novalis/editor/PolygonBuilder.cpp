#include "PolygonBuilder.h"

#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

nv::editor::PolygonBuilder::PolygonBuilder(SDL_Renderer* renderer) noexcept
	: m_renderer{ renderer }
{
}

std::optional<nv::Polygon> nv::editor::PolygonBuilder::operator()(SDL_Point mouse) noexcept {
	ImGui::SetMouseCursor(ImGuiMouseCursor_None);

	if (!m_polygon.rep.outer().empty()) {
		m_firstPoint.point = toSDLPoint(m_polygon.rep.outer()[0]);
		m_firstPoint.render(m_renderer);
		m_lastPlacedPoint.point = mouse;
		m_lastPlacedPoint.render(m_renderer);
		m_polygon.render(m_renderer);
	}

	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
		if (m_firstPoint.containsCoord(mouse) && m_polygon.rep.outer().size() > 1) {
			auto ret = m_polygon;
			m_polygon.rep.outer().clear();
			building = false;
			return ret;
		} else {
			auto placedPoint = m_polygon.rep.outer().emplace_back(mouse.x, mouse.y); //add a new point
			m_lastPlacedPoint.point = { placedPoint.get<0>(), placedPoint.get<1>() };
		}
	}

	return std::nullopt;
}