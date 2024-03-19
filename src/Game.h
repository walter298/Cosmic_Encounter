#pragma once

#include <iostream>
#include <fstream>
#include <random>
#include <string_view>

#include <csv.h>

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

template<typename CardType>
struct Deck {
private:
	using Cards = std::vector<CardType>;

	Cards m_cards;
	Cards::iterator m_firstCard = m_cards.begin();
	Cards::iterator m_discardPileBorder = m_cards.begin();
	Cards::iterator m_discardPileEnd = m_cards.begin();

	template<typename... Args>
	void parse(std::string_view fileName, Args&... args) {
		std::ifstream file{ fileName.data() };
		assert(file.is_open());

		auto parse = [](std::istringstream& iss, auto& arg) {
			using ArgType = std::remove_reference_t<decltype(arg)>;
			if constexpr (std::is_enum_v<ArgType>) {
				typename std::underlying_type_t<ArgType> underlying;
				iss >> underlying;
				arg = static_cast<ArgType>(underlying);
			} else {
				iss >> arg;
			}
		};

		std::string line;
		std::istringstream iss;
		while (std::getline(file, line)) {
			iss.str(line);
			int occC = 0;
			iss >> occC;
			((parse(iss, args)), ...);
			for (int i = 0; i < occC; i++) {
				m_cards.emplace_back(args...);
			}
		}

		file.close();
	}

	Deck(size_t cardC) {
		m_cards.reserve(cardC * 2); //reserve double size because we store discard pile in same vector
		m_discardPileBorder = m_cards.begin() + cardC; //set discard pile beginnning in middle of vector
	}
public:
	Deck(std::string_view fileName, size_t cardC) requires(std::is_aggregate_v<CardType>) : Deck(cardC) {
		auto applyWrapper = [&fileName, this](auto&... args) {
			parse(fileName, args...);
		};
		CardType argSrc;
		aggregateApply(applyWrapper, argSrc);
	}
	Deck(std::string_view fileName, size_t cardC) : Deck(cardC) {
		CardType cardSrc;
		parse(fileName, cardSrc);
	}

	void shuffleDiscardBackIn() {
		std::ranges::shuffle(std::ranges::subrange(m_firstCard, m_cards.end()));
		m_discardPileBorder = m_cards.end();
		m_discardPileEnd = m_cards.end();
	}

	void draw(Cards& hand, size_t cardC) {
		if (m_firstCard == m_discardPileBorder) {
			shuffleDiscardBackIn();
		}
		auto finalCardToDrawIt = m_firstCard + cardC;
		hand.insert(hand.end(), m_firstCard, finalCardToDrawIt);
		m_firstCard = finalCardToDrawIt;
	}

	CardType discardTop() {
		if (m_firstCard == m_discardPileBorder) {
			shuffleDiscardBackIn();
		}
		auto top = *m_firstCard;
		m_discardPileEnd++; //extend discard pile
		*m_discardPileEnd = top; //move top card to end of discard pile
		return top;
	}

	void shuffle(std::mt19937& gen) {
		std::ranges::shuffle(std::ranges::subrange(m_firstCard, m_discardPileBorder), gen); 
	}
};

std::string toString(Card card);

constexpr inline size_t COLOR_C = 5;

struct Colony {
	using ShipCounts = std::array<int, COLOR_C>;
	ShipCounts ships{ 0, 0, 0, 0, 0 };
};
using Systems = std::array<std::vector<Colony>, COLOR_C>;

//used to index into Planet::ships and Game::Systems
enum Color : size_t {
	Red,
	Blue,
	Green,
	Black,
	Purple
};

struct Player {
	Client& cli;
	std::vector<Card> hand;
	Color color;
};

using Players = std::vector<Player>;

struct GameState {
	Systems systems;

	Players players;

	Deck<Card> deck{ "cosmic_deck.csv", 40 };
	Deck<Color> destinyDeck{ "destiny_deck.csv", 15 };
};

void play(size_t pCount, tcp::endpoint&& endpoint);