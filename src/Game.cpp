#include "Game.h"

Colonies makeStartingColonies(Color pColor) {
	Colonies ret(5);
	std::ranges::generate(ret, [&] {
		Colony colony;
		colony.ships[pColor] = 4;
		return colony;
	});
	return ret;
}

Player::Player(tcp::socket&& sock, Color color, size_t index)
	: sock{ std::move(sock) }, color{ color }, colonies{ makeStartingColonies(color) }, index{ index }
{
}
