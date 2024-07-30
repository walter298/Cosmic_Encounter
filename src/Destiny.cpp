#include "Destiny.h"

#include "novalis/Physics.h"

#include "Game.h"

namespace {
	constexpr int DRAWN_COLOR_LAYER = 4;
	constexpr int DESTINY_BACK_LAYER = 5;

	nv::Rect& chooseDestiny(Socket& sock, nv::Scene& mainUi, const ColorMap& colors, bool takingTurn) {
		Color drawnColor;
		sock.read(drawnColor);

		auto colorRect = colors.at(drawnColor).second;
		auto& colorBack = mainUi.findRef(mainUi.textureRefs[0], "destiny_back");
		colorRect.setPos(colorBack.getPos());
		colorRect.setSize(colorBack.getSize());

		auto& rects = mainUi.rects[DRAWN_COLOR_LAYER];
		rects.push_back(colorRect);
		return rects.back();
	}
}

void showDestiny(Socket& sock, nv::Scene& mainUi, const ColorMap& colors, bool takingTurn) {
	mainUi.eventHandler.addPeriodicEvent([&, colorDrawn = false]() mutable {
		if (!colorDrawn) {
			colorDrawn = true;

			using namespace std::literals;

			mainUi.eventHandler.addPeriodicEvent(nv::MoveScheduler{
				chooseDestiny(sock, mainUi, colors, takingTurn),
				nv::PixelRate{ 15, 20ms },
				nv::PixelRate{ 0, 0ms },
				400,
				0
			});
		}
	});

	mainUi();
}
