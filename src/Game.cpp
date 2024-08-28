#include "Game.h"

Player::Player(tcp::socket&& sock, Color color, size_t index)
	: sock{ std::move(sock) }, color{ color }, index{ index }
{
}
