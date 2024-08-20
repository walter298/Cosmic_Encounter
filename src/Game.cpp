#include "Game.h"

bool Card::operator==(const Card& other) const noexcept {
	return value == other.value && type == other.type;
}

Player::Player(tcp::socket&& sock, Color color, size_t index)
	: sock{ std::move(sock) }, color{ color }, index{ index }
{
}
