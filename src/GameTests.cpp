#include "GameTests.h"

#include <ranges>

#include "novalis/data_util/BasicJsonSerialization.h"
#include "novalis/Instance.h"

#include "Alliance.h"
#include "NetworkUtil.h"
#include "ShowAlienSystem.h"

namespace {
	GameRenderData makeExampleGameRenderData(SDL_Renderer* renderer) {
		GameRenderData ret{ renderer };

		ret.fontMap.emplace("Arial", nv::loadFont(nv::relativePath("Cosmic_Encounter/fonts/arial/ARIAL.TTF"), 45));
		
		auto makeTexture = [&](std::string_view path) {
			return nv::Texture{
				ret.renderer,
				nv::parseJson(path),
				ret.texMap
			};
		};

		auto makeColorRect = [](Uint8 r, Uint8 g, Uint8 b) {
			return nv::Rect{ 0, 0, 0, 0, r, g, b };
		};

		ret.colorMap[Red] = std::pair{ 
			makeTexture("Cosmic_Encounter/game_assets/texture_objects/virus.nv_texture_object"),
			makeColorRect(255, 0, 0) 
		};
		ret.colorMap[Green] = std::pair{
			makeTexture("Cosmic_Encounter/game_assets/texture_objects/pacifist.nv_texture_object"),
			makeColorRect(0, 255, 0)
		};

		return ret;
	}
}

void testPlanetSelector() {
	nv::Instance instance{ "Planet Selection" };
	
	auto gameRenderData = makeExampleGameRenderData(instance.renderer);

	ShowAlienSystem showAlienSystem{ instance.renderer, gameRenderData.texMap, gameRenderData.fontMap };

	PlanetSelector planetSelector{ showAlienSystem };
	Colonies colonies(5);

	std::ranges::generate(colonies, [] {
		Colony ret;
		ret.ships[Red]   = 5;
		ret.ships[Green] = -343;
		ret.ships[Black] = 56;
		return ret;
	});
	planetSelector(instance.renderer, Black, std::ranges::subrange(colonies.begin(), colonies.end()), gameRenderData.colorMap);
}

void testAlliance() {
	nv::Instance instance{ "Alliances" };
	
	auto gameRenderData = makeExampleGameRenderData(instance.renderer);

	Alliance alliance{ gameRenderData };
	alliance(Red, Green, true, true);
}

struct Person {
	std::string name;
	int age = 0;
};

static std::vector<Socket> sockets;

constexpr inline std::string_view DELIM = "#########!!!!!!!!!";

//class Socket2 {
//private:
//	tcp::socket m_sock;
//	
//	std::vector<std::byte> m_inputBuff;
//	size_t m_firstByteIndex = 0;
//	size_t m_extraBytesSent = 0;
//
//	std::vector<std::byte> m_outputBuff;
//public:
//	struct Error {
//		sys::error_code ec;
//		std::source_location loc;
//	};
//private:
//	Error m_lastError;
//public:
//	template<typename Executor>
//	explicit Socket2(Executor& exec) : m_sock{ exec }
//	{
//	}
//
//	explicit Socket2(tcp::socket&& sock) noexcept : m_sock{ std::move(sock) }
//	{
//	}
//
//	asio::awaitable<void> asyncConnect(const tcp::endpoint& endpoint, sys::error_code& ec) noexcept {
//		co_await m_sock.async_connect(endpoint, asio::redirect_error(asio::use_awaitable, ec));
//	}
//
//	void checkError(sys::error_code error, std::source_location current) noexcept {
//		if (error) {
//			std::println("Socket Error: {} in {}", error.message(), current.function_name());
//			m_lastError = { error, current };
//			assert(false);
//		}
//	}
//
//	void checkError(zpp::bits::errc error, std::source_location current) noexcept {
//		if (zpp::bits::failure(error)) {
//			m_lastError = { std::make_error_code(error), current };
//			std::println("Socket Error: {} in {}", m_lastError.ec.message(), current.function_name());
//			assert(false);
//		}
//	}
//private:
//	template<typename... Args>
//	auto serializeImpl(Args&&... args) noexcept {
//		zpp::bits::out serializer{ m_outputBuff, zpp::bits::endian::network() };
//		checkError(serializer(std::forward<Args>(args)...), std::source_location::current());
//		checkError(serializer(DELIM), std::source_location::current());
//		return asio::buffer(serializer.processed_data().data(), serializer.processed_data().size());
//	}
//public:
//	template<typename... Args>
//	void send(Args&&... args) noexcept {
//		auto serializedDataBuff = serializeImpl(std::forward<Args>(args)...);
//		asio::write(m_sock, serializedDataBuff);
//		m_outputBuff.clear();
//	}
//	template<typename... Args>
//	asio::awaitable<void> asyncSend(Args&&... args) noexcept {
//		assert(m_sock.is_open());
//		auto serializedDataBuff = serializeImpl(std::forward<Args>(args)...);
//		sys::error_code ec;
//		auto bytesWritten = co_await asio::async_write(m_sock, serializedDataBuff, asio::redirect_error(asio::use_awaitable, ec));
//		checkError(ec, std::source_location::current());
//		m_outputBuff.clear();
//	}
//private:
//	template<typename... Args>
//	void deserializeImpl(size_t bytesRead, Args&... args) noexcept {
//		zpp::bits::in deserializer{ m_inputBuff, zpp::bits::endian::network() };
//		checkError(deserializer(args...), std::source_location::current());
//	}
//public:
//	template<typename... Args>
//	void read(Args&... args) noexcept {
//		sys::error_code ec;
//		auto bytesRead = asio::read_until(m_sock, asio::dynamic_buffer(m_inputBuff), DELIM, ec);
//		checkError(ec, std::source_location::current());
//		deserializeImpl(bytesRead, args...);
//	}
//	template<typename... Args>
//	asio::awaitable<void> asyncRead(Args&... args) noexcept {
//		assert(m_sock.is_open());
//		sys::error_code ec;
//		auto bytesRead = co_await asio::async_read_until(m_sock, asio::dynamic_buffer(m_inputBuff),
//			DELIM, asio::redirect_error(asio::use_awaitable, ec));
//		checkError(ec, std::source_location::current());
//		m_inputBuff.resize(m_inputBuff.size() - DELIM.size()); //make sure we don't parse the delimer
//		deserializeImpl(bytesRead, args...);
//	}
//};

static asio::awaitable<void> sendMessageImpl(asio::io_context& context, const tcp::endpoint& endpoint) {
	Socket client{ context };

	co_await client.asyncConnect(endpoint);
	
	std::vector<Person> people;
	for (int i = 0; i < 500; i++) {
		people.emplace_back("John", i);
	}

	auto subrange = std::ranges::subrange(people.begin(), people.begin() + 200);
	
	auto start = std::chrono::system_clock::now();

	for (int i = 0; i < 200; i++) {
		SocketHeader msg{};
		co_await client.asyncSend(msg, subrange);
		auto temp = subrange;
		co_await client.asyncRead(msg, temp);
	}
	
	std::println("Subrange echo took {}", std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now() - start)
	);
}

static asio::awaitable<void> recvMessageImpl(asio::io_context& context, const tcp::endpoint& endpoint) {
	tcp::acceptor acceptor{ context, endpoint };

	sys::error_code ec;
	auto tempServerSock = co_await acceptor.async_accept(asio::redirect_error(asio::use_awaitable, ec));
	if (ec) {
		std::println("{}", ec.message());
	}
	Socket server{ std::move(tempServerSock) };

	for (int i = 0; i < 200; i++) {
		std::vector<Person> people;
		SocketHeader msg{};
		co_await server.asyncRead(msg, people);
		co_await server.asyncSend(msg, people);
	}
}

void testSocketIO() {
	asio::io_context context;
	tcp::endpoint serverEndpoint{ ip::make_address("192.168.7.250"), 5555 };

	asio::co_spawn(context, sendMessageImpl(context, serverEndpoint), asio::detached);
	asio::co_spawn(context, recvMessageImpl(context, serverEndpoint), asio::detached);

	asio::io_context::work work{ context };
	context.run();
}

void benchmarkArenaAllocation() {

}
