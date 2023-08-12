#pragma once

#include <functional>
#include <vector>

#include <nlohmann/json.hpp>

#include "novalis/RenderTypes.h"

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