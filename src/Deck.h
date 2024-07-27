#pragma once

#include <algorithm>
#include <cassert>
#include <fstream>
#include <random>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>

#include <boost/pfr.hpp>

template<typename CardType>
struct Deck {
private:
	std::random_device& m_rbg;

	using Cards = std::vector<CardType>;

	Cards m_cards;
	Cards::iterator m_firstCard = m_cards.begin();

	Deck(size_t cardC, std::random_device& rbg) : m_rbg{ rbg } {
		m_cards.reserve(cardC); //reserve double size because we store discard pile in same vector
	}
public:
	Deck(std::string_view fileName, size_t cardC, std::random_device& rbg) : Deck(cardC, rbg) {
		std::ifstream file{ fileName.data() };
		assert(file.is_open());

		std::string line;
		std::istringstream iss;

		auto parseMember = [&](auto& arg) {
			using ArgType = std::remove_reference_t<decltype(arg)>;
			if constexpr (std::is_enum_v<ArgType>) {
				typename std::underlying_type_t<ArgType> underlying;
				iss >> underlying;
				arg = static_cast<ArgType>(underlying);
			}
			else {
				iss >> arg;
			}
		};

		auto addCards = [&, this]() {
			iss.str(line);
			int occC = 0;
			iss >> occC;
			CardType newCard{};
			boost::pfr::for_each_field(newCard, parseMember);
			for (int i = 0; i < occC; i++) {
				m_cards.push_back(newCard);
			}
			iss.clear();
		};

		while (std::getline(file, line)) {
			addCards();
		}

		m_firstCard = m_cards.begin();

		shuffle();
	}

	Deck(std::random_device& rbg) : m_rbg{ rbg } {}

	Deck& operator=(Cards&& cards) {
		m_cards = std::move(cards);
		m_firstCard = m_cards.begin();
		return *this;
	}

	void shuffleDiscardBackIn() {
		m_firstCard = m_cards.begin();
		shuffle();
	}

	void draw(Cards& hand, size_t cardC) {
		for (size_t i = 0; i < cardC; i++) {
			hand.push_back(discardTop());
		}
	}

	CardType discardTop() {
		if (m_firstCard == m_cards.end()) {
			shuffleDiscardBackIn();
		}
		auto& top = *m_firstCard;
		m_firstCard++;
		return top;
	}

	void shuffle() {
		std::ranges::shuffle(std::ranges::subrange(m_firstCard, m_cards.end()), m_rbg);
	}
};