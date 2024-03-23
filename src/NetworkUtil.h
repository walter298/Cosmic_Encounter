#pragma once

#include <algorithm>
#include <functional>
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
namespace sys = boost::system;

constexpr inline std::string_view MSG_END_DELIM{ "###" };
constexpr inline std::string_view MSG_ERROR{ "ERROR" };
constexpr inline char DATUM_END_CHR = '!';
constexpr inline char DATUM_BREAK_CHR = '$';
constexpr inline char DATUM_ELEM_BREAK_CHR = '&';

using StringIt = std::string::const_iterator;
std::string nextDatum(StringIt& begin, StringIt end);

template<typename T>
concept Aggregate = std::is_aggregate_v<std::remove_cvref_t<T>>;

namespace detail {
	template<Aggregate T, size_t... MemberIdxs>
	void parseEachMember(std::string& data, T& aggr, std::index_sequence<MemberIdxs...> seq) {
		auto begin = data.begin();
		((parseValueFromString(nextDatum(begin, data.end()), pfr::get<MemberIdxs>(aggr))), ...);
	}
}

template<Aggregate T> 
void parseValueFromString(std::string& str, T& aggr) {
	detail::parseEachMember(str, aggr, std::make_index_sequence<pfr::tuple_size_v<T>>());
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
void parseValueFromString(const std::string& str, Integral& integral) {
	integral = boost::lexical_cast<Integral>(str);
}

template<std::ranges::viewable_range Range>
void parseValueFromString(const std::string& str, Range& range) {
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

template<typename T>
concept Enum = std::is_enum_v<std::remove_cvref_t<T>>;

template<Enum Enm>
void parseValueFromString(const std::string& str, Enm& enm) {
	using UnderylingType = std::underlying_type_t<Enm>;
	UnderylingType num{};
	parseValueFromString(str, num);
	enm = static_cast<Enm>(num);
}

template<typename Buffer>
void readMsg(Buffer&) = delete; //you have to pass in at least one argument

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
		parseValueFromString(str, arg);
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
decltype(auto) toString(String&& str) { //no need to parse string
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

template<Enum Enm>
auto toString(Enm enm) {
	using UnderlyingType = std::underlying_type_t<Enm>;
	return toString(static_cast<UnderlyingType>(enm));
}

template<typename... Args>
auto writeMsg(Args&&... args) {
	std::string buffer;
	((buffer.append(toString(std::forward<Args>(args)) + '!')), ...);
	buffer.append(MSG_END_DELIM);
	return buffer;
}

template<typename T>
struct FunctionTraits { //primary template assumes function call operator
	using args = FunctionTraits<decltype(&T::operator())>::args;
};

template<typename R, typename... Args>
struct FunctionTraits<R(Args...)> { //specialization for functions that haven't decayed
	using args = std::tuple<Args...>;
};

template<typename R, typename... Args>
struct FunctionTraits<R(*)(Args...)> { //specialization for function pointers
	using args = std::tuple<Args...>;
};

template<typename C, typename R, typename... Args>
struct FunctionTraits<R(C::*)(Args...) const> { //specialization for const member functions
	using args = std::tuple<Args...>;
};

template<typename C, typename R, typename... Args>
struct FunctionTraits<R(C::*)(Args...)> { //specialization for mutable member functions
	using args = std::tuple<Args...>;
};

template<template<typename> typename Trait>
struct ApplyTrait {
	template<typename T>
	struct Apply;

	template<typename... Ts>
	struct Apply<std::tuple<Ts...>> {
		using type = typename std::tuple<typename Trait<Ts>::type...>;
	};
};

class Socket {
private:
	tcp::socket m_sock;
	std::string m_msgBeingSent;
	std::string m_msgBeingRcvd;
	
	using ReconnCB = std::move_only_function<void(sys::error_code ec, tcp::socket&)>;

	ReconnCB m_whenDisconnected{ [](auto ec, auto& sock) { std::cerr << ec << '\n'; } };
	ReconnCB m_asyncWhenDisconnected{ [](auto ec, auto& sock) { std::cerr << ec << '\n'; } };
public:
	explicit Socket(asio::io_context& context);
	explicit Socket(tcp::socket&& sock);
	Socket(tcp::socket&& sock, ReconnCB&& whenDisconnected, ReconnCB&& asyncWhenDsiconnected);

	void onDisconnected(ReconnCB&& cb);
	void onAsyncDisconnected(ReconnCB&& cb);

	void connect(const tcp::endpoint& endpoint);
	void disconnect();

	template<typename... Args>
	void read(Args&... args) {
		while (true) {
			sys::error_code ec;
			while (true) {
				m_msgBeingRcvd.clear();
				asio::read_until(m_sock, asio::dynamic_buffer(m_msgBeingRcvd), MSG_END_DELIM, ec);
				if (ec) {
					m_whenDisconnected(ec, m_sock);
				} else {
					break;
				}
			}
		}
		readMsg(m_msgBeingRcvd, args...);
	}
	template<typename... Args>
	asio::awaitable<void> asyncRead(Args&... args) {
		sys::error_code ec;
		while (true) {
			m_msgBeingRcvd.clear();
			co_await asio::async_read_until(m_sock, asio::dynamic_buffer(m_msgBeingRcvd), MSG_END_DELIM,
				asio::redirect_error(asio::use_awaitable, ec));
			if (ec) {
				co_await m_asyncWhenDisconnected(ec, m_sock);
			} else {
				break;
			}
		}
		readMsg(m_msgBeingRcvd, args...);
	}
	
	void send(const std::string& msg);
	asio::awaitable<void> asyncSend(const std::string& msg);
};

class Server {
private:
	asio::io_context m_context;
	asio::io_context::strand m_strand;
	tcp::acceptor m_acceptor;
	std::vector<Socket> m_clients;

	void reconnect(sys::error_code ec, tcp::socket& socket);
	void asyncReconnect(sys::error_code ec, tcp::socket& sock);

	asio::awaitable<void> acceptConnection();

	template<typename Func>
	asio::awaitable<void> asyncRecvFromClient(Func argsFunc, Socket& cli) {
		using Args = ApplyTrait<std::remove_cvref>::Apply<typename FunctionTraits<Func>::args>::type;
		Args args;
		co_await std::apply(&Socket::asyncRead, cli, args);
		std::apply(argsFunc, args);
	}
public:
	Server(const tcp::endpoint& endpoint);
	
	void acceptConnections(size_t connC);

	Socket& getClient(size_t idx);

	void broadcast(const std::string& msg);

	template<typename Func>
	void readFromAll(Func f) {
		for (auto& client : m_clients) {
			asio::co_spawn(m_context, asyncRecvFromClient(f, client), asio::detached);
		}
		m_context.run();
	}
};
