#include "Server.h"

#include <iostream> //cin
#include <print>

#include "Game.h"
#include "Lobby.h"

namespace ranges = std::ranges;
namespace views = std::views;

namespace {
	template<ranges::viewable_range PlayerRange, typename... Args>
	void broadcast(PlayerRange&& players, Args&&... args) {
		std::tuple argsTuple{ std::forward<Args>(args)... };
		for (auto&& player : players) {
			std::apply([&](const auto&... tupleArgs) {
				std::unwrap_reference_t<Player&>(player).sock.send(tupleArgs...);
			}, argsTuple);
		}
	}

	template<std::ranges::viewable_range Range, std::convertible_to<size_t>... Idxs>
	auto excludeElems(Range& range, Idxs... idxs) {
		return range | views::enumerate | views::filter([&](auto&& elemIdxPair) {
			return ((std::get<0>(elemIdxPair) != idxs) && ...);
		}) | views::transform([](auto&& elemIdxPair) {
			return std::ref(std::get<1>(elemIdxPair));
		});
	}
}

static bool acceptedOwnColorBeingDrawn(Player& turnTaker, size_t& colonyIdx) {
	if (std::ranges::any_of(turnTaker.colonies,
		[](const auto& colony) { return colony.hasEnemyShips; })) {
		bool accepted = false;
		Color color{};
		turnTaker.sock.read(accepted, color, colonyIdx);
		return accepted;
	} else {
		return false;
	}
}

struct Defense {
	Player* player;
	size_t colonyIdx = 0;
};

static Defense getDefense(Player& turnTaker, GameState& gameState) {
	Player* defensivePlayer = nullptr;
	size_t colonyIdx = 0;

	while (true) {
		auto colorDrawn = gameState.destinyDeck.discardTop();
		broadcast(gameState.players, colorDrawn);

		if (colorDrawn != turnTaker.color) { //player does not have a choice to keep drawing if another color is drawn
			defensivePlayer = &*ranges::find_if(gameState.players, [&](const auto& player) { 
				return player.color == turnTaker.color; 
			});
			break;
		} else if (acceptedOwnColorBeingDrawn(turnTaker, colonyIdx)) {
			break;
		}
	}
	return { defensivePlayer, colonyIdx };
}

static Players acceptConnections(size_t pCount, tcp::acceptor& acceptor, std::random_device& rbg) {
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
		auto& player = players.emplace_back(Socket{ acceptor.accept() }, availableColors[i]);
		
		/*send the newly joined player his or her color and alien,
		and the colors and aliens of each other player who has already joined*/
		player.sock.send(pCount, pRenderDataV | views::take(i + 1));

		//send each other player the new color and alien
		broadcast(players | views::take(i), pRenderDataV[i]);
	}
	
	return players;
}

static std::vector<Color> loadDestinyDeck(const Players& players) {
	std::vector<Color> colors{ players.size() * 3 };
	size_t playerIdx = 0;

	ranges::generate(colors, [&]() {
		auto color = players[playerIdx].color;
		playerIdx += ((playerIdx + 1) % players.size());
		return color;
	});

	return colors;
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
	broadcast(gameState.players, STARTING_GAME);

	//send each player his or her 8 cards and the turn order
	for (auto& player : gameState.players) {
		gameState.deck.draw(player.hand, 8);
		player.sock.send(player.hand, gameState.players | views::transform([](const auto& player) {
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