#pragma once

#include <boost/bimap.hpp>

#include "Novalis/Instance.h"
#include "novalis/Scene.h"

#include "Card.h"

void associateCardRenders(nv::NovalisInstance& instance);

std::vector<nv::TextPtr> getCardRenders(const std::vector<Card>& cards);

void formatCards(std::vector<nv::TextPtr>& cards, int x, int x2, int y);

class CardSelection : public nv::Scene {
private:
	std::vector<nv::TextPtr> m_cards;

	Card m_selectedCard;

	void checkSelectedCard();
public:
	CardSelection(nv::NovalisInstance& instance, Cards& cards);

	Card selectedCard() const noexcept;
};