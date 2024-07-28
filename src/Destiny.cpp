#include "Destiny.h"

#include "novalis/Physics.h"

#include "Game.h"

constexpr int DRAWN_COLOR_IDX = 4;
constexpr int DESTINY_BACK_LAYER = 5;

static nv::Rect& chooseDestiny(Socket& sock, nv::Scene& mainUi, const ColorMap& colors, bool takingTurn) {
	mainUi.textures[DRAWN_COLOR_IDX].clear();

	Color drawnColor;
	sock.read(drawnColor);

	auto colorRect = colors.at(drawnColor);
	auto& colorBackObj = mainUi.findRef(mainUi.textureRefs[0], "destiny_back");
	colorRect.setPos(colorBackObj.getPos());
	colorRect.setSize(colorBackObj.getSize());

	auto& rects = mainUi.rects[DRAWN_COLOR_IDX];
	rects.push_back(colorRect);
	return rects.back();
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
