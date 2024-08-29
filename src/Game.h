#pragma once

#include <random>

#include <boost/container/static_vector.hpp>

#include "novalis/data_util/File.h"

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
};

constexpr inline size_t COLOR_C = 5;

//used to index into Planet::ships and Game::Systems (DO NOT CHANGE THE ORDER OF VALUES!!)
enum Color : size_t {
	Black,
	Blue,
	Green,
	Purple,
	Red
};

struct Colony {
	using ShipCounts = boost::unordered_flat_map<Color, int>;
	ShipCounts ships;
	bool hasEnemyShips = false;
};
using Colonies = boost::container::static_vector<Colony, 10>;

Colonies makeStartingColonies(Color pColor);

//offer messages for destiny stage
constexpr inline char OWN_SYSTEM   = 'a';
constexpr inline char OTHER_SYSTEM = 'b';

enum Alien : size_t {
	Pacifist,
	Virus,
	Laser,
	Oracle,
	Zombie
};

struct Player {
	Player(tcp::socket&& sock, Color color, size_t index);

	Socket sock;
	Color color;
	std::vector<Card> hand;
	Colonies colonies;
	size_t index = 0;
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