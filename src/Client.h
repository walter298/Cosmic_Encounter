#pragma once

#include <asio/ip/tcp.hpp>

#include "CardRender.h"
#include "Network.h"

using asio::ip::tcp;

struct Client {
	std::string name;
	MessageHandler handler;

	Player p;

	ConnectionState joinGame(asio::io_context& context, const tcp::endpoint& serverEndpoint);
};
