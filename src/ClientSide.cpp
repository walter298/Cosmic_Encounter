#include "ClientSide.h"

void play(const tcp::endpoint& endpoint) {
	asio::io_context context;
	Socket cli{ context };

	nv::Instance instance{ "Cosmic Encounter" };
	
	nv::Scene scene{ instance };
	
}