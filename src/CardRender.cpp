#include "CardRender.h"

using CardRenderBimap = boost::bimap<Card, nv::Text*>;
CardRenderBimap cardRenders;

void associateCardRenders(nv::NovalisInstance& instance) {
	Cards deck = makeDeck();
	deck.erase(std::unique(deck.begin(), deck.end()), deck.end()); //remove duplicates

	for (const auto& card : deck) {
		std::string name;
		switch (card.type) {
		case Card::Type::Negotiate:
			name = "negotiate";
			break;
		case Card::Type::Attack:
			name = std::to_string(card.value) + "_attack";
			break;
		case Card::Type::Reinforcement:
			name = std::to_string(card.value) + "_reinforcement";
			break;
		}
		cardRenders.insert(CardRenderBimap::value_type(card, instance.getObj<nv::Text>(name)));
	}
}

std::vector<nv::TextPtr> getCardRenders(const Cards& cards) {
	std::vector<nv::TextPtr> ret;
	ret.reserve(cards.size());
	for (const auto& card : cards) {
		ret.push_back(nv::copyRenderObj(cardRenders.left.at(card)));
	}
	return ret;
}

void formatCards(std::vector<nv::TextPtr>& cards, int x, int x2, int y)
{
	constexpr int cardPixelGap  = 25;
	const int totalPixelGap = static_cast<int>(std::ssize(cards) - 1) * cardPixelGap;

	const int maxTotalCardWidth = (x2 - x) - totalPixelGap;
	const int cardWidth  = maxTotalCardWidth / static_cast<int>(std::ssize(cards));
	const int cardHeight = cardWidth / 4;
	
	for (auto& card : cards) {
		card->setSize(cardWidth, cardHeight);
		card->setRenPos(x, y);
		x += (cardWidth + cardPixelGap + 75);
	}
}

void CardSelection::checkSelectedCard()
{
	if (nv::InputHandler::getInstance().leftMouseClicked()) {
		auto [mx, my] = nv::InputHandler::getInstance().mouse();
		for (const auto& card : m_cards) {
			if (card->backgroundRect().isCoordContained(mx, my)) {
				//m_selectedCard = cardRenders.right.at(card.get());
				//endScene(EndReason::NextScene);
			}
		}
	}
}

CardSelection::CardSelection(nv::NovalisInstance& instance, Cards& cards)
	: Scene(nv::workingDirectory() + "/scenes/card_selection.txt", instance)
{
	m_cards = getCardRenders(cards);
	if (cards.size() > 0) {
		formatCards(m_cards, 325, 1200, 1000);
	}

	for (const auto& card : m_cards) {
		addObj(card.get());
	}
	addEvent([this] { checkSelectedCard(); });
}