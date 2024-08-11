#include "Destiny.h"

#include "novalis/Physics.h"

#include "Game.h"

namespace {
	constexpr int BUTTON_LAYER = 0;
	constexpr int DRAWN_COLOR_LAYER = 4;
	constexpr int DESTINY_BACK_LAYER = 5;

	SDL_Point acceptColorRectPos{ 0, 0 };
	SDL_Point acceptColorTextPos{ 0, 0 };

	SDL_Point keepDrawingRectPos{ 0, 0 };
	SDL_Point keepDrawingTextPos{ 0, 0 };
	
	bool showingButton = false;

	auto getButtons(nv::Scene& mainUi) {
		auto& acceptButtonRect = mainUi.find<nv::RectRef>(BUTTON_LAYER, "accept_button_rect").get();
		auto& acceptButtonText = mainUi.find<nv::TextureRef>(BUTTON_LAYER, "accept_button_text").get();
		auto& keepDrawingButtonRect = mainUi.find<nv::RectRef>(BUTTON_LAYER, "keep_drawing_button_rect").get();
		auto& keepDrawingButtonText = mainUi.find<nv::TextRef>(BUTTON_LAYER, "keep_drawing_button_text").get();
		
		return std::tie(acceptButtonRect, acceptButtonText, keepDrawingButtonRect, keepDrawingButtonText);
	}

	auto toggleButtons(nv::Scene& mainUi, bool showing) {
		auto [acceptButtonRect, acceptButtonText, keepDrawingButtonRect, keepDrawingButtonText] = getButtons(mainUi);

		constexpr SDL_Point OUT_OF_SCENE{ 5000, 5000 };

		if (showing) {
			acceptButtonRect.setPos(acceptColorRectPos);
			acceptButtonText.setPos(acceptColorTextPos);
			keepDrawingButtonRect.setPos(keepDrawingRectPos);
			keepDrawingButtonText.setPos(keepDrawingTextPos);
		} else {
			acceptButtonRect.setPos(OUT_OF_SCENE);
			acceptButtonText.setPos(OUT_OF_SCENE);
			keepDrawingButtonRect.setPos(OUT_OF_SCENE);
			keepDrawingButtonText.setPos(OUT_OF_SCENE);
		}

		return std::tie(acceptButtonRect, keepDrawingButtonRect);
	}

	std::pair<nv::ID, nv::ID> initButtons(Socket& sock, nv::Scene& mainUi) {
		auto [acceptButtonRect, keepDrawingButtonRect] = toggleButtons(mainUi, true);

		auto makeButton = [&](nv::Rect& rect, int code) {
			return mainUi.addEvent(nv::Button{
				acceptButtonRect,
				[&] { sock.send(code); mainUi.running = false; },
				[&] { rect.setRenderColor(34, 139, 34, 255); },
				[&] { rect.setRenderColor(255, 255, 255, 255); }
			});
		};

		return std::pair{
			makeButton(acceptButtonRect, ACCEPTED_COLOR),
			makeButton(keepDrawingButtonRect, DECIDED_TO_KEEP_DRAWING)
		};
	}

	nv::Rect& readDrawnColor(Socket& sock, nv::Scene& mainUi, const ColorMap& colors, Color pColor, bool takingTurn) {
		Color drawnColor;
		sock.read(drawnColor);

		//toggleButtons(mainUi, pColor == drawnColor && takingTurn);

		auto colorRect = colors.at(drawnColor).second;
		auto& colorBack = mainUi.find<nv::TextureRef>(0, "destiny_back").get();
		colorRect.setPos(colorBack.getPos());
		colorRect.setSize(colorBack.getSize());

		return mainUi.addObject(colorRect, DESTINY_BACK_LAYER);
	}
}

void showDestiny(Socket& sock, nv::Scene& mainUi, const ColorMap& colors, Color pColor, bool takingTurn) {
	bool colorDrawn = false;
	mainUi.addEvent([&] {
		if (!colorDrawn) {
			colorDrawn = true;

			using namespace std::literals;

			mainUi.addEvent(nv::MoveScheduler{
				readDrawnColor(sock, mainUi, colors, pColor, takingTurn),
				nv::PixelRate{ 15, 20ms },
				nv::PixelRate{ 0, 0ms },
				400,
				0
			});
		}
	});

	mainUi();
	mainUi.deoverlay();
}
