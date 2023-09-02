#include "CardRender.h"

std::map<Card, nv::Text> cardRenders;

void associateCardRenders(nv::Instance& instance) {
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
		cardRenders.emplace(card, instance.getText(name));
	}
}

std::vector<nv::Text> copyCardRenders(const Cards& cards) {
	std::vector<nv::Text> ret;
	ret.reserve(cards.size());
	for (const auto& card : cards) {
		ret.push_back(cardRenders.at(card));
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
		for (const auto& [cardRender, card] : m_cardMap) {
			/*if (cardRender.getBackgroundRect().isCoordContained(mx, my)) {
				m_selectedCard = card;
				endScene(EndReason::NextScene);
			}*/
		}
	}
}

CardSelection::CardSelection(nv::Instance& instance, const Cards& cards)
	: Scene(nv::workingDirectory() + "/scenes/card_selection.txt", instance), 
	m_cardRenders(copyCardRenders(cards))
{
	auto setCardPos = [this, x = 0](nv::Text& cardRender) mutable {
		cardRender.setRenPos(x, 700);
		x += (cardRender.backgroundRect().rect.w + 20);
		render(&cardRender, 1);
	};
	for (auto [idx, cardRender] : std::views::enumerate(m_cardRenders)) {
		m_cardMap[idx] = cards[idx];
		setCardPos(cardRender);
	}
	
	addEvent([this] { checkSelectedCard(); });
}