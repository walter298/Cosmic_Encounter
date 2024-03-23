#include "Game.h"

std::string toString(Card card) {
	return toString(static_cast<int>(card.type)) + '!' + toString(card.attackValue) + '!';
}