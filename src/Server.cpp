#include "Server.h"

#include <iostream>
#include <print>

#include <magic_enum.hpp>

#include "novalis/data_util/Views.h"

#include "Destiny.h"
#include "Game.h"
#include "Lobby.h"

namespace ranges = std::ranges;
namespace views = std::views;

namespace {
	template<typename PlayerRange, typename... Args>
	void broadcast(PlayerRange&& players, SocketHeader msg, Args&&... args) {
		std::tuple argsTuple{ std::forward<Args>(args)... };
		for (auto& player : players) {
			std::apply([&](const auto&... tupleArgs) {
				nv::unrefwrap(player).sock.send(msg, tupleArgs...);
			}, argsTuple);
		}
	}

	struct Defense {
		Player* player = nullptr;
		size_t colonyIdx = 0;
	};

	bool hasToKeepRedrawing(Player& turnTaker, GameState& gameState, Defense& defense) {
		auto colorDrawn = gameState.destinyDeck.discardTop();
		
		//get the defensive player
		defense.player = &(*std::ranges::find_if(gameState.players, [&](const auto& player) {
			return player.color == turnTaker.color;
		}));

		if (colorDrawn != turnTaker.color) { //player does not have a choice to keep drawing if another color is drawn
			broadcast(gameState.players, SocketHeader::DESTINY_DRAW_INFO, DestinyDrawInfo{ DestinyDrawOptions::MustChoose, colorDrawn });
			return false;// we are not redrawing a mandatory color
		} else if (std::ranges::any_of(turnTaker.colonies, [](const auto& colony) { return colony.hasEnemyShips; })) {
			broadcast(gameState.players, SocketHeader::DESTINY_DRAW_INFO, DestinyDrawInfo{ DestinyDrawOptions::CanRedrawOrChoose, colorDrawn });
			DestinyDrawChoice resp;
			turnTaker.sock.read(SocketHeader::DESTINY_DRAW_CHOICE, resp);
			return resp == DestinyDrawChoice::DecidedToKeepDrawing;
		} else {
			broadcast(gameState.players, SocketHeader::DESTINY_DRAW_INFO, DestinyDrawInfo{ DestinyDrawOptions::MustRedraw, colorDrawn });
			return true; //turn-taker MUST redraw if own color is drawn but system doesn't have enemy ships
		}
	}

	Defense getDefense(Player& turnTaker, GameState& gameState) {
		Defense defense;

		while (hasToKeepRedrawing(turnTaker, gameState, defense)) {}

		turnTaker.sock.send(SocketHeader::COLONY_INFORMATION, turnTaker.colonies);
		turnTaker.sock.read(SocketHeader::CHOSEN_COLONY, defense.colonyIdx);

		std::cin.get();
		exit(-1); //this is where the game ends

		broadcast(nv::ExcludeIndices(gameState.players, turnTaker.index), SocketHeader::CHOSEN_COLONY, defense.colonyIdx);

		return defense;
	}
	
	Players acceptConnections(size_t pCount, tcp::acceptor& acceptor, std::random_device& rbg) {
		std::println("Waiting for players to join");

		assert(pCount < 6);

		Players players;
		players.reserve(pCount);

		std::array availableColors = { Red, Green, Blue, Black, Purple };
		std::array availableAliens = { Pacifist, Virus, Pacifist, Virus, Pacifist };

		ranges::shuffle(availableColors, rbg);
		ranges::shuffle(availableAliens, rbg);

		//combine availableColors and availableAliens
		std::vector<PlayerRenderData> pRenderDataV;
		pRenderDataV.append_range(
			ranges::zip_view(availableAliens, availableColors) |
			views::transform([](auto&& pair) {
				return PlayerRenderData{ std::get<0>(pair), std::get<1>(pair) };
			})
		);

		//accept connections
		for (size_t i = 0; i < pCount; i++) {
			auto& player = players.emplace_back(acceptor.accept(), availableColors[i], i);

			/*send the newly joined player his or her color and alien,
			and the colors and aliens of each other player who has already joined*/
			for (const auto& pRenderData : pRenderDataV | views::take(i + 1)) {
				std::println("Sending: {} {}", magic_enum::enum_name(pRenderData.alien), magic_enum::enum_name(pRenderData.color));
			}
			player.sock.send(SocketHeader::ALREADY_ARRIVED_PLAYERS, pCount, pRenderDataV | views::take(i + 1));

			//send each other player the new color and alien
			broadcast(players | views::take(i), SocketHeader::NEWLY_JOINED_PLAYER, pRenderDataV[i]);
		}

		return players;
	}

	std::vector<Color> loadDestinyDeck(const Players& players) {
		std::vector<Color> colors{ players.size() * 3 };
		size_t playerIdx = 0;

		ranges::generate(colors, [&] {
			auto color = players[playerIdx].color;
			playerIdx += ((playerIdx + 1) % players.size());
			return color;
		});

		return colors;
	}
}

void host(size_t pCount, const tcp::endpoint& endpoint) {
	asio::io_context ctx;
	
	//run networking asynchronously
	std::jthread ctxThread{ [&] { 
		asio::io_context::work work{ ctx };
		ctx.run(); 
	}};

	tcp::acceptor acceptor{ ctx, endpoint };
	
	GameState gameState;
	
	gameState.players = acceptConnections(pCount, acceptor, gameState.rbg);

	std::println("Press enter to start the game");
	std::cin.get();

	//tell everyone the game is starting
	broadcast(gameState.players, SocketHeader::STARTING_GAME);

	//send each player his 8 cards and the turn order
	for (auto& player : gameState.players) {
		gameState.deck.draw(player.hand, 8); //draw 8 cards from the deck
		player.sock.send(SocketHeader::CARDS_AND_TURN_ORDER, player.hand, gameState.players | views::transform([](const auto& player) {
			return player.color;
		}));
	}

	gameState.destinyDeck = loadDestinyDeck(gameState.players);

	size_t turnTakerIdx = 0;

	//game loop
	while (true) {
		auto& turnTaker = gameState.players[turnTakerIdx];
		auto defense = getDefense(turnTaker, gameState);
	}
}