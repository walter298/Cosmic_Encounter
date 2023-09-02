#pragma once

#include <ranges>
#include <unordered_map>

#include <boost/bimap.hpp>

#include "Novalis/Instance.h"
#include "novalis/Scene.h"

#include "GameState.h"

void associateCardRenders(nv::Instance& instance);

std::vector<nv::Text> copyCardRenders(const Cards& cards);

void formatCards(std::vector<nv::TextPtr>& cards, int x, int x2, int y);

class CardSelection : public nv::Scene {
private:
	std::vector<nv::Text> m_cardRenders;
	std::unordered_map<size_t, Card> m_cardMap;
	Card m_selectedCard;

	void checkSelectedCard();
public:
	CardSelection(nv::Instance& instance, const Cards& cards);

	Card selectedCard() const noexcept;
};