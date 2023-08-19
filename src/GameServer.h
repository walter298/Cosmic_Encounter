#pragma once

#include <algorithm>
#include <array>
#include <chrono>
#include <deque>
#include <iostream>
#include <random>
#include <ranges>

#include "ClientSide.h"
#include "GameState.h"
#include "Network.h"

namespace rg = std::ranges;

template<typename Con1, typename Con2>
constexpr void moveTopContentsOut(Con1& dest, Con2& src, size_t elemCount) {
	dest.insert(dest.end(),
		std::make_move_iterator(src.begin()),
		std::make_move_iterator(src.begin() + elemCount)
	);
	src.erase(src.begin(), src.begin() + elemCount);
}

class GameServer {
private:
	Server m_server;
	std::map<Color, Client> m_players;
	Cards m_deck, m_discardPile;

	void addPlayer(tcp::socket&& socket, const Color& color);
public:
	GameServer(asio::io_context& context, tcp::endpoint endpoint);
	void waitForPlayers(size_t pCount, std::error_code& ec);
	void setup();
};