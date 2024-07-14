#pragma once

#include <algorithm>
#include <functional>
#include <print>
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
void readMsg(Buffer& buffer, Args&... args) {
	std::println("Reading this message: {}", buffer);

	auto buffBegin = std::begin(buffer);

	auto parseStr = [&](auto& arg) {
		auto msgEnd = std::ranges::find(buffBegin, std::end(buffer), DATUM_END_CHR);
		if (msgEnd == std::end(buffer)) {
			std::println("{} has invalid message end", buffer);
			throw std::exception{ "bad message" };
		}
	
		std::string str{ std::make_move_iterator(buffBegin), std::make_move_iterator(msgEnd) };
		std::println("Current substring: {}", str);
		parseValueFromString(str, arg);

		buffBegin = std::next(msgEnd);
	};

	((parseStr(args)), ...);
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
std::string toString(const Range& range) {
	std::string ret;
	for (const auto& elem : range) {
		ret.append(toString(elem) + '!');
	}
	return ret;
}

template<Enum Enm>
std::string toString(Enm enm) {
	using UnderlyingType = std::underlying_type_t<Enm>;
	return toString(static_cast<UnderlyingType>(enm));
}

template<Aggregate Aggr>
std::string toString(Aggr& aggr) {
	std::string buff;
	pfr::for_each_field(aggr, [&](const auto& field) {
		buff.push_back(toString(field));
	});
	return buff;
}

template<typename... Args>
void writeMsg(std::string& buff, Args&&... args) {
	((buff.append(toString(std::forward<Args>(args)) + '!')), ...);
	buff.append(MSG_END_DELIM);
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

	ReconnCB m_reconnect{ [](auto ec, auto& sock) { std::println("{}", ec.message()); } };
	ReconnCB m_asyncReconnect{ [](auto ec, auto& sock) { std::println("{}", ec.message()); } };
public:
	template<typename Executor>
	explicit Socket(Executor& exec) : m_sock{ exec } 
	{
	}
	Socket(tcp::socket&& sock);
	
	asio::any_io_executor getExecutor();

	void setReconnection(ReconnCB&& cb);
	void setAsyncReconnection(ReconnCB&& cb);

	bool connect(const tcp::endpoint& endpoint, sys::error_code& ec);
	void disconnect();

	template<typename... Args>
	void read(Args&... args) {
		sys::error_code ec;
		while (true) {
			m_msgBeingRcvd.clear();
			asio::read_until(m_sock, asio::dynamic_buffer(m_msgBeingRcvd), MSG_END_DELIM, ec);
			std::println("Parsing {}", m_msgBeingRcvd);
			if (ec) {
				std::println("Reconnecting because: {}", ec.message());
				m_reconnect(ec, m_sock);
			} else {
				break;
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
				co_await m_asyncReconnect(ec, m_sock);
			}
			else {
				break;
			}
		}
		readMsg(m_msgBeingRcvd, args...);
	}
	
	template<typename... Args>
	void send(Args&... args) {
		m_msgBeingSent.clear();
		writeMsg(m_msgBeingSent, args...);
		sys::error_code ec;
		while (true) {
			asio::write(m_sock, asio::buffer(m_msgBeingSent), ec);
			if (ec) {
				m_reconnect(ec, m_sock);
			} else {
				break;
			}
		}
	}

	template<typename... Args>
	asio::awaitable<void> asyncSend(Args&... args) {
		m_msgBeingSent = writeMsg(args...);
		sys::error_code ec;
		while (true) {
			co_await asio::async_write(m_sock, asio::buffer(m_msgBeingRcvd),
				asio::redirect_error(asio::use_awaitable, ec));
			if (ec) {
				m_asyncReconnect(ec, m_sock);
			} else {
				break;
			}
		}
	}
};