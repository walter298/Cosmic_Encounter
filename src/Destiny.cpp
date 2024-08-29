#include "Destiny.h"

#include "novalis/Physics.h"

#include "Game.h"

namespace {
	constexpr int RECT_LAYER = 2;
	constexpr int TEXT_LAYER = 3;
	constexpr int DRAWN_COLOR_LAYER = 4;
	constexpr int DESTINY_BACK_LAYER = 1;

	auto getColorRectMover(nv::Scene& ui, Color drawnColor, const ColorMap& colorMap) {
		auto colorRect = std::get<1>(colorMap.at(drawnColor));
		auto& colorBack = ui.find<nv::Texture>(DESTINY_BACK_LAYER, "destiny_back").get();
		colorRect.setPos(colorBack.getPos());
		colorRect.setSize(colorBack.getSize());

		using namespace std::literals;

		return nv::MoveScheduler{
			ui.addObject(colorRect, DRAWN_COLOR_LAYER).get(),
			nv::PixelRate{ 15, 20ms },
			nv::PixelRate{ 55, 50ms },
			400,
			0
		};
	}
}

asio::awaitable<void> NonTurnTakingDestiny::asyncReadColorsFromServer() {
	while (true) {
		//get the color drawn 
		DestinyDrawInfo destiny;
		co_await m_sock.asyncRead(destiny);

		std::unique_lock lock{ m_mutex };
		m_drawnColors.push_back(destiny.drawnColor);

		if (destiny.drawChoice == CanRedrawOrChoose) {
			lock.unlock(); //IMPORTANT: unlock mutex so we don't block while waiting for turn-taker response
			DestinyResponseFromTurnTaker turnTakerResp{};
			co_await m_sock.asyncRead(turnTakerResp);
			if (turnTakerResp == AcceptedColor) {
				lock.lock();
				m_wasFinalColorSent = true;
				break;
			}
		}
	}
}

NonTurnTakingDestiny::NonTurnTakingDestiny(Socket& sock, SDL_Renderer* renderer, nv::TextureMap& texMap,
	nv::FontMap& fontMap, const ColorMap& colorMap) 
	: m_sock{ sock }, 
	m_scene { nv::relativePath("Cosmic_Encounter/game_assets/scenes/non_turn_taking_destiny.nv_scene"), renderer, texMap, fontMap }
{
	m_scene.addEvent([&, this] {
		std::scoped_lock lock{ m_mutex };
		if (!m_drawnColors.empty() && !m_wasFinalColorSent) {
			m_scene.addEvent(getColorRectMover(m_scene, m_drawnColors.back(), colorMap));
			m_drawnColors.pop_back();
		} else if (m_wasFinalColorSent) {
			m_scene.addEvent(nv::EventChain{
				getColorRectMover(m_scene, m_drawnColors.back(), colorMap),
				[&] { m_scene.running = false; return true; }
			});
		}
	});
}

void NonTurnTakingDestiny::operator()() {
	m_drawnColors.clear();
	m_wasFinalColorSent = false;
	asio::co_spawn(m_sock.getExecutor(), asyncReadColorsFromServer(), asio::detached);
	m_scene();
}

void TurnTakingDestiny::readDrawnColor(nv::Rect& acceptButtonRect, nv::Text& acceptButtonText, nv::Rect& keepDrawingButtonRect,
	nv::Text& keepDrawingButtonText, const ColorMap& colorMap) {
	DestinyDrawInfo destinyResult;
	m_sock.read(destinyResult);

	m_scene.addEvent(getColorRectMover(m_scene, destinyResult.drawnColor, colorMap));
	m_currColor = destinyResult.drawnColor;

	switch (destinyResult.drawChoice) {
	case MustRedraw:
		toggleButton(acceptButtonRect, acceptButtonText, false);
		toggleButton(keepDrawingButtonRect, keepDrawingButtonText, true);
		break;
	case MustChoose:
		toggleButton(acceptButtonRect, acceptButtonText, true);
		toggleButton(keepDrawingButtonRect, keepDrawingButtonText, false);
		break;
	case CanRedrawOrChoose:
		toggleButton(acceptButtonRect, acceptButtonText, true);
		toggleButton(keepDrawingButtonRect, keepDrawingButtonText, true);
		break;
	}
}

void TurnTakingDestiny::toggleButton(nv::Rect& rect, nv::Text& text, bool showing) {
	static const auto originalRectPos = rect.getPos();
	static const auto originalTextPos = text.getPos();
	static constexpr SDL_Point OUT_OF_SCENE{ 5000, 5000 };

	if (showing) {
		rect.setPos(originalRectPos);
		text.setPos(originalTextPos);
	} else {
		rect.setPos(OUT_OF_SCENE);
		text.setPos(OUT_OF_SCENE);
	}
}

TurnTakingDestiny::TurnTakingDestiny(Socket& sock, SDL_Renderer* renderer, nv::TextureMap& texMap,
	nv::FontMap& fontMap, const ColorMap& colorMap)
	: m_sock{ sock },
	m_scene{ nv::relativePath("Cosmic_Encounter/game_assets/scenes/turn_taking_destiny.nv_scene"), renderer, texMap, fontMap }
{
	//button to accept the color being drawn
	auto& acceptButtonRect = m_scene.find<nv::Rect>(RECT_LAYER, "accept_button_rect").get();
	auto& acceptButtonText = m_scene.find<nv::Text>(TEXT_LAYER, "accept_button_text").get();

	//map accept and reject buttons to their respective rects
	m_scene.addEvent(nv::Button{
		nv::usingExternalRect,
		acceptButtonRect,
		[this] { m_sock.send(AcceptedColor); m_scene.running = false; },
		[&] { acceptButtonRect.setRenderColor(34, 139, 34, 255); },
		[&] { acceptButtonRect.setRenderColor(34, 139, 34, 255); }
	});

	//by default, redrawing colors is not an option
	auto& keepDrawingButtonRect = m_scene.find<nv::Rect>(RECT_LAYER, "keep_drawing_button_rect").get();
	auto& keepDrawingButtonText = m_scene.find<nv::Text>(TEXT_LAYER, "keep_drawing_button_text").get();
	toggleButton(keepDrawingButtonRect, keepDrawingButtonText, false);

	m_scene.addEvent(nv::Button{
		nv::usingExternalRect,
		keepDrawingButtonRect,
		[this] { m_sock.send(DecidedToKeepDrawing); m_wasColorDrawn = false; },
		[&] { keepDrawingButtonRect.setRenderColor(34, 139, 34, 255); },
		[&] { keepDrawingButtonRect.setRenderColor(255, 255, 255,255); }
	});
	m_scene.addEvent([&] {
		if (!m_wasColorDrawn) {
			m_wasColorDrawn = true;
			readDrawnColor(
				acceptButtonRect,
				acceptButtonText,
				keepDrawingButtonRect,
				keepDrawingButtonText,
				colorMap
			);
		}
	});
}

Color TurnTakingDestiny::operator()() {
	m_wasColorDrawn = false;
	m_scene();
	return m_currColor;
}