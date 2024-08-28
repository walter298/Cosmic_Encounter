#include "GameTests.h"

#include "novalis/Instance.h"

#include "ShowAlienSystem.h"

void testPlanetSelector() {
	nv::Instance instance{ "Planet Selection" };
	nv::TextureMap texMap;
	nv::FontMap fontMap;

	fontMap.emplace("Arial", nv::loadFont(nv::relativePath("Cosmic_Encounter/fonts/arial/ARIAL.TTF"), 45));
	ColorMap colorMap;
	
	ShowAlienSystem showAlienSystem{ instance.renderer, texMap, fontMap };

	auto makeColorRect = [](Uint8 r, Uint8 g, Uint8 b) {
		return nv::Rect{ nullptr, 0, 0, 0, 0, r, g, b };
	};
	colorMap[Red]   = std::pair{ showAlienSystem.planets.getTextures()[0], makeColorRect(255, 0, 0) };
	colorMap[Green] = std::pair{ showAlienSystem.planets.getTextures()[0], makeColorRect(0, 255, 0) };
	colorMap[Black] = std::pair{ showAlienSystem.planets.getTextures()[0], makeColorRect(139, 131, 131) };

	PlanetSelector planetSelector{ showAlienSystem };
	Colonies colonies(5);

	ranges::generate(colonies, [] {
		Colony ret;
		ret.ships[Red] = 5;
		ret.ships[Green] = -343;
		ret.ships[Black] = 56;
		return ret;
	});
	planetSelector(Black, ranges::subrange(colonies.begin(), colonies.end()), colorMap);
}

void benchmarkArenaAllocation() {

}
