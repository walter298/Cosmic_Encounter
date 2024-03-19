#pragma once

#include <algorithm>
#include <iostream>
#include <ranges>
#include <string>
#include <type_traits>
#include <vector>

#define BOOST_ASIO_DISABLE_HANDLER_TYPE_REQUIREMENTS

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/pfr.hpp>

namespace asio = boost::asio;
namespace ip = asio::ip;
using ip::tcp;
namespace pfr = boost::pfr;

constexpr inline std::string_view MSG_END_DELIM{ "###" };
constexpr inline std::string_view MSG_ERROR{ "ERROR" };
constexpr inline char DATUM_END_CHR = '!';
constexpr inline char DATUM_BREAK_CHR = '$';
constexpr inline char DATUM_ELEM_BREAK_CHR = '&';

using StringIt = std::string::iterator;
std::string nextDatum(StringIt& begin, StringIt end);

template<typename T>
concept Aggregate = std::is_aggregate_v<std::remove_cvref_t<T>>;

template<Aggregate T, size_t... MemberIdxs>
void parseEachMember(std::string& data, T& aggr, std::index_sequence<MemberIdxs...> seq) {
	auto begin = data.begin();
	((parseValueFromString(nextDatum(begin, data.end()), pfr::get<MemberIdxs>(aggr))), ...);
}

template<Aggregate T> 
void parseValueFromString(std::string& str, T& aggr) {
	parseEachMember(str, aggr, std::make_index_sequence<pfr::tuple_size_v<T>>());
}

namespace detail {
	template<typename Callable, Aggregate T, size_t... MemberIdxs>
	decltype(auto) aggregateApplyImpl(Callable&& callable, T&& aggr, std::index_sequence<MemberIdxs...> seq) {
		return std::invoke(std::forward<Callable>(callable), pfr::get<MemberIdxs>(std::forward<T>(aggr))...);
	}
}

template<typename Callable, Aggregate T>
decltype(auto) aggregateApply(Callable&& callable, T&& aggr) {
	return detail::aggregateApplyImpl(std::forward<Callable>(callable), std::forward<T>(aggr),
		std::make_index_sequence<pfr::tuple_size_v<std::remove_cvref_t<T>>>());
}

template<std::integral Integral>
void parseValueFromString(std::string& str, Integral integral) {
	return boost::lexical_cast<Integral>(str);
}

template<std::ranges::viewable_range Range>
void parseValueFromString(std::string& str, Range& range) {
	auto begin = str.begin();
	while (true) {
		auto nextElemBreak = std::ranges::find(begin, str.end());
		if (nextElemBreak == str.end()) {
			break;
		}
		typename Range::element_type elem{};
		range.insert(range.end(), parseValueFromString(str, elem));
	}
}

template<std::ranges::viewable_range Buffer, typename... Args>
bool readMsg(Buffer& buffer, Args&... args) {
	auto buffBegin = std::begin(buffer);

	namespace rngs = std::ranges;
	auto parseStr = [&](auto& arg) {
		if (buffBegin == std::end(buffer)) {
			return false;
		}
		auto msgEnd = rngs::find(buffBegin, std::end(buffer), DATUM_END_CHR);
		if (msgEnd == std::end(buffer)) {
			return false;
		}
		
		auto nextBuffBegin = std::next(msgEnd);
		std::string str{ std::make_move_iterator(buffBegin), std::make_move_iterator(msgEnd) };
		//parseValueFromString(str, arg);
		buffBegin = nextBuffBegin;

		return true;
	};

	return (parseStr(args) && ...);
}

template<std::integral Num>
std::string toString(Num num) {
	return std::to_string(num);
}
template<std::convertible_to<std::string> String>
decltype(auto) toString(String&& str) {
	return std::forward<String>(str);
}
template<std::ranges::viewable_range Range>
auto toString(const Range& range) {
	std::string ret;
	for (const auto& elem : range) {
		ret.append(toString(elem) + '!');
	}
	return ret;
}

template<typename... Args>
auto writeMsg(Args&&... args) {
	std::string buffer;
	((buffer.append(toString(std::forward<Args>(args)) + '!')), ...);
	buffer.append(MSG_END_DELIM);
	return buffer;
}

struct Client {
	tcp::socket sock;
	tcp::endpoint endpoint;
	std::string msgBeingSent;
	std::string msgBeingRcvd;
};

class Server {
private:
	asio::io_context m_context;
	asio::io_context::strand m_strand;
	tcp::acceptor m_acceptor;
	std::vector<Client> m_clients;

	asio::awaitable<void> reconnectPlayer(Client& player);
	asio::awaitable<void> acceptConnection();

	asio::awaitable<void> sendToClient(const std::string& msg, Client& cli);
public:
	Server(tcp::endpoint&& endpoint);
	void acceptConnections(int connC);

	Client& getClient(size_t idx);

	template<std::same_as<Client>... Clients> 
	void sendToClients(const std::string& msg, Clients&... clients) {
		((asio::co_spawn(m_context, msg, asio::detached), ...);
		m_context.run();
	}

	template<std::invocable<Client&> MessageGenerator>
	void sendToPlayers(MessageGenerator msgGen) {
		boost::system::error_code ec;
		for (auto& cli : m_clients) {
			asio::co_spawn(m_context, sendToClient(cli, msgGen), asio::detached);
		}
		m_context.run();
	}
};
