#include "GameState.h"

bool Card::operator<(const Card& other) const noexcept {
	return value < other.value;
}

bool Card::operator==(const Card& other) const noexcept {
	return (value == other.value) && (type == other.type);
}

void to_json(json& j, const Card& card) {
	j["type"] = card.type;
	if (card.type == Card::Type::Attack || card.type == Card::Type::Reinforcement) {
		j["value"] = card.value;
	}
	//todo: deal with artifact cards
}

void from_json(const json& j, Card& card) {
	j.at("type").get_to(card.type);
	if (card.type == Card::Type::Attack || card.type == Card::Type::Reinforcement) {
		j.at("value").get_to(card.value);
	}
	//todo: deal with artifact cards
}

Cards makeDeck()
{
	auto makeAttack = [](int attackVal) {
		return Card{
			Card::Type::Attack,
			attackVal
		};
	};
	auto makeReinforcement = [](int reinforceVal) {
		Card card;
		card.value = reinforceVal;
		return card;
	};
	auto makeNegotiate = [] {
		return Card{
			Card::Type::Negotiate
		};
	};

	std::vector<Card> deck;
	deck.reserve(72);

	//add 15 negotiates
	for (int i = 0; i < 15; i++) {
		deck.push_back(makeNegotiate());
	}

	//add attack cards
	deck.push_back(makeAttack(0));                                  //1x attack 00
	deck.push_back(makeAttack(1));                                  //1x attack 01
	for (int i = 0; i < 4; i++) { deck.push_back(makeAttack(4)); }  //4x attack 04
	deck.push_back(makeAttack(5));                                  //1x attack 05
	for (int i = 0; i < 7; i++) { deck.push_back(makeAttack(7)); }  //7x attack 07
	for (int i = 0; i < 7; i++) { deck.push_back(makeAttack(8)); }  //7x attack 08
	deck.push_back(makeAttack(9));                                  //1x attack 09
	for (int i = 0; i < 4; i++) { deck.push_back(makeAttack(10)); } //4x attack 10
	deck.push_back(makeAttack(11));                                 //1x attack 11
	for (int i = 0; i < 2; i++) { deck.push_back(makeAttack(12)); } //2x attack 12
	for (int i = 0; i < 2; i++) { deck.push_back(makeAttack(14)); } //2x attack 14
	deck.push_back(makeAttack(15));                                 //1x attack 15
	for (int i = 0; i < 2; i++) { deck.push_back(makeAttack(20)); } //2x attack 20
	deck.push_back(makeAttack(23));                                 //1x attack 23
	deck.push_back(makeAttack(30));                                 //1x attack 30
	deck.push_back(makeAttack(40));                                 //1x attack 40

	//todo: deal with reinforcements
	//todo: deal with artifact cards and the morph

	return deck;
}