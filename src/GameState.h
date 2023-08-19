#pragma once

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00 
#endif

#include <functional>
#include <map>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

enum class Phase {
	Setup,
	Start,
	Regroup,
	Desiny,
	Launch,
	Alliance,
	Planning,
	Reveal,
	Resolution
};

struct Card {
public:
	enum class Type {
		Attack,
		Reinforcement,
		Negotiate,
		Morph,
		Artifact
	};

	Type type = Type::Attack;
	int value = 0;

	bool operator<(const Card& other) const noexcept;
	bool operator==(const Card& other) const noexcept;

	//for artifact cards only
	std::vector<Phase> playablePhases;
	std::function<void()> effect;
};

using nlohmann::json;

void to_json(json& json, const Card& card);
void from_json(const json& json, Card& card);

using Cards = std::vector<Card>;

//IT MATTERS that the cards in pushed into the deck in sorted order of value!
Cards makeDeck();

enum class Color {
	Red,
	Green,
	Blue,
	Yellow,
	Purple,
	Black
};

struct Ship {
	int val = 1;
};

using Ships = std::map<Color, std::vector<Ship>>;

struct Planet {
	Ships ships;
};

struct Player {
	Cards hand;
	int shipCount = 20;
	Color color = Color::Red;
};