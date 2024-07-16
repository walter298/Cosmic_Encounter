#include "Game.h"

bool Card::operator==(const Card& other) const noexcept {
	return value == other.value && type == other.type;
}
