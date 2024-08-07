#pragma once

#include <fstream>
#include <random>
#include <string_view>
#include <unordered_map>

#include <boost/functional/hash.hpp>

#include "novalis/DataUtil.h"

#include "Deck.h"
#include "NetworkUtil.h"

struct Card {
	enum Type : size_t {
		Attack, 
		Negotiate,
		Reinforcement
	};
	Type type = Type::Attack;
	int value = 0;

	bool operator==(const Card& other) const noexcept;
};

namespace std {
	template<>
	struct hash<Card> {
		size_t operator()(const Card& card) const {
			size_t ret = 0;
			boost::hash_combine(ret, card.type);
			boost::hash_combine(ret, card.value);
			return ret;
		}
	};
}

constexpr inline size_t COLOR_C = 5;

struct Colony {
	using ShipCounts = std::array<int, COLOR_C>;
	ShipCounts ships{ 0, 0, 0, 0, 0 };
	bool hasEnemyShips = false;
};
using Colonies = std::vector<Colony>;

//offer messages for destiny stage
constexpr inline char OWN_SYSTEM = 'a';
constexpr inline char OTHER_SYSTEM = 'b';

//used to index into Planet::ships and Game::Systems
enum Color : size_t {
	Red,
	Blue,
	Green,
	Black,
	Purple
};

enum Alien : size_t {
	Pacifist,
	Virus,
	Laser,
	Oracle,
	Zombie
};

struct Player {
	Socket sock;
	Color color;
	std::vector<Card> hand;
	std::array<Colony, 5> colonies;
};

using Players = std::vector<Player>;

struct GameState {
	std::random_device rbg;
	Players players;
	Deck<Card> deck{ nv::relativePath("Cosmic_Encounter/cosmic_deck.csv"), 40, rbg };
	Deck<Color> destinyDeck{ rbg };
};

//[from, to] is inclusive
template<std::integral Integral>
static Integral randomNum(Integral from, Integral to) {
	thread_local std::random_device dev;
	thread_local std::mt19937 gen{ dev };
	std::uniform_int_distribution dist{ from, to };
	return dist(gen());
}