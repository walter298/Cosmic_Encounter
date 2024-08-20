#include "Destiny.h"

#include <atomic>

#include "novalis/Physics.h"

#include "Game.h"

namespace {
	constexpr int RECT_LAYER = 2;
	constexpr int TEXT_LAYER = 3;
	constexpr int DRAWN_COLOR_LAYER = 4;
	constexpr int DESTINY_BACK_LAYER = 1;

	auto getAcceptButton(nv::Scene& mainUi) {
		static auto& acceptButtonRect = mainUi.find<nv::Rect>(RECT_LAYER, "accept_button_rect");
		static auto& acceptButtonText = mainUi.find<nv::Text>(TEXT_LAYER, "accept_button_text");
		return std::tie(acceptButtonRect, acceptButtonText);
	}
	auto getKeepDrawingButton(nv::Scene& mainUi) {
		static auto& keepDrawingButtonRect = mainUi.find<nv::Rect>(RECT_LAYER, "keep_drawing_button_rect");
		static auto& keepDrawingButtonText = mainUi.find<nv::Text>(TEXT_LAYER, "keep_drawing_button_text");
		return std::tie(keepDrawingButtonRect, keepDrawingButtonText);
	}
	void toggleButton(nv::Rect& rect, nv::Text& text, bool showing) {
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

	auto getColorRectMover(nv::Scene& ui, Color drawnColor, const ColorMap& colorMap) {
		auto colorRect = colorMap.at(drawnColor).second;
		auto& colorBack = ui.find<nv::Texture>(DESTINY_BACK_LAYER, "destiny_back");
		colorRect.setPos(colorBack.getPos());
		colorRect.setSize(colorBack.getSize());

		using namespace std::literals;

		auto obj = ui.addObject(colorRect, DRAWN_COLOR_LAYER);
		return nv::MoveScheduler{
			obj.get(),
			nv::PixelRate{ 15, 20ms },
			nv::PixelRate{ 55, 50ms },
			400,
			0
		};
	}

	void readDrawnColorForTurnTaker(Socket& sock, nv::Scene& ui, const ColorMap& colors, nv::Rect& acceptButtonRect,
		nv::Text& acceptButtonText, nv::Rect& keepDrawingButtonRect, nv::Text& keepDrawingButtonText) 
	{
		TurnTakerDestinyMessage destinyResult;
		sock.read(destinyResult);

		ui.addEvent(getColorRectMover(ui, destinyResult.drawnColor, colors));

		if (destinyResult.mustKeepDrawing) {
			toggleButton(acceptButtonRect, acceptButtonText, false);
			toggleButton(keepDrawingButtonRect, keepDrawingButtonText, true);
		} else if (destinyResult.allowedToKeepDrawing) {
			toggleButton(acceptButtonRect, acceptButtonText, true);
			toggleButton(keepDrawingButtonRect, keepDrawingButtonText, true);
		} else {
			toggleButton(acceptButtonRect, acceptButtonText, true);
			toggleButton(keepDrawingButtonRect, keepDrawingButtonText, false);
		}
	}

	void showDestinyForTurnTaker(Socket& sock, nv::Scene& ui, const ColorMap& colorMap) {
		//button to accept the color being drawn
		auto [acceptButtonRect, acceptButtonText] = getAcceptButton(ui);

		//map accept and reject buttons to their respective rects
		auto acceptBtnID = ui.addEvent(nv::Button{
			nv::usingExternalRect,
			acceptButtonRect,
			[&] { sock.send(AcceptedColor); ui.running = false; },
			[&] { acceptButtonRect.setRenderColor(34, 139, 34, 255); },
			[&] { acceptButtonRect.setRenderColor(34, 139, 34, 255); }
		});

		//by default, redrawing colors is not an option
		auto [keepDrawingButtonRect, keepDrawingButtonText] = getKeepDrawingButton(ui);
		toggleButton(keepDrawingButtonRect, keepDrawingButtonText, false);

		bool wasColorDrawn = false;

		auto keepDrawingBtnID = ui.addEvent(nv::Button{
		nv::usingExternalRect,
		keepDrawingButtonRect,
			[&] { sock.send(DecidedToKeepDrawing); wasColorDrawn = false; },
			[&] { keepDrawingButtonRect.setRenderColor(34, 139, 34, 255); },
			[&] { keepDrawingButtonRect.setRenderColor(34, 139, 34, 255); }
		});
		auto cardMoveEvtID = ui.addEvent([&] {
			if (!wasColorDrawn) {
				wasColorDrawn = true;
				readDrawnColorForTurnTaker(sock, ui, colorMap, acceptButtonRect, acceptButtonText, keepDrawingButtonRect, keepDrawingButtonText);
			}
		});
		ui();
		ui.removeEvent(keepDrawingBtnID);
		ui.removeEvent(cardMoveEvtID);
	}

	std::mutex mutex;

	asio::awaitable<void> asyncReadColorsFromServer(Socket& sock, std::vector<Color>& drawnColors, bool& wasFinalColorSent) {
		while (true) {
			NonTurnTakerDestinyMessage destiny;
			co_await sock.asyncRead(destiny);
			
			std::scoped_lock lock{ mutex };
			drawnColors.push_back(destiny.drawnColor);

			if (destiny.finalColor) {
				wasFinalColorSent = true;
				break;
			}
		} 
	}

	void showDestinyForNonTurnTaker(Socket& sock, nv::Scene& ui, const ColorMap& colorMap) {
		bool wasFinalColorSent = false;
		std::vector<Color> drawnColors;
		asio::co_spawn(sock.getExecutor(), asyncReadColorsFromServer(sock, drawnColors, wasFinalColorSent), asio::use_awaitable);

		ui.addEvent([&] {
			std::unique_lock lock{ mutex };
			if (!drawnColors.empty() && !wasFinalColorSent) {
				ui.addEvent(getColorRectMover(ui, drawnColors.back(), colorMap));
				drawnColors.pop_back();
			} else if (wasFinalColorSent) {
				ui.addEvent(nv::EventChain{
					getColorRectMover(ui, drawnColors.back(), colorMap),
					[&] { ui.running = false; return true; }
				});
			}
		});
		ui();
	}
}

void showDestiny(Socket& sock, SDL_Renderer* renderer, nv::TextureMap& texMap, nv::FontMap& fontMap, const ColorMap& colorMap, bool takingTurn) {
	if (takingTurn) {
		static nv::Scene ui{ nv::relativePath("Cosmic_Encounter/game_assets/scenes/turn_taking_destiny.nv_scene"), renderer, texMap, fontMap };
		showDestinyForTurnTaker(sock, ui, colorMap);
	} else {
		static nv::Scene ui{ nv::relativePath("Cosmic_Encounter/game_assets/scenes/non_turn_taking_destiny.nv_scene"), renderer, texMap, fontMap };
		showDestinyForNonTurnTaker(sock, ui, colorMap);
	}
}
