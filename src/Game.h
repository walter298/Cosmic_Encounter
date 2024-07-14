#pragma once

#include <fstream>
#include <random>
#include <string_view>

#include <boost/pfr.hpp>

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
	int attackValue = 0;
};

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

class GameState {
private:
	std::random_device m_dev;
public:
	std::mt19937 rbg{ m_dev() };
	Players players;
	Deck<Card> deck{ nv::relativePath("Cosmic_Encounter/cosmic_deck.csv"), 40, rbg };
	Deck<Color> destinyDeck{ nv::relativePath("Cosmic_Encounter/destiny_deck.csv"), 15, rbg };
};

//[from, to] is inclusive
template<std::integral Integral>
static Integral randomNum(Integral from, Integral to) {
	thread_local std::random_device dev;
	thread_local std::mt19937 gen{ dev };
	std::uniform_int_distribution dist{ from, to };
	return dist(gen());
}