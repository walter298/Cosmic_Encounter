#pragma once

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00 
#endif

#include <string>
#include <map>

#include <asio.hpp>

#include "Network.h"
#include "Card.h"

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
	const std::string name;
	Cards hand;
	int shipCount = 20;

	Player(MessageHandler&& msgHandler, std::string name);
	Player(asio::io_context& context, std::string name);

	MessageHandler msgHandler;
	MessageHandler* operator->();

	ConnectionState joinGame(asio::io_context& context, const tcp::endpoint& serverEndpoint);
};