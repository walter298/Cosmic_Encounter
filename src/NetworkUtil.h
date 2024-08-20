#pragma once

#include <algorithm>
#include <functional>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include <SDKDDKVer.h> //windows version macros for boost.asio

#define BOOST_ASIO_DISABLE_HANDLER_TYPE_REQUIREMENTS

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/pfr.hpp>

namespace ranges = std::ranges;

//boost namespace aliases
namespace asio = boost::asio;
namespace ip = asio::ip;
using ip::tcp;
namespace pfr = boost::pfr;
namespace sys = boost::system;

constexpr inline char MSG_END_DELIM = '#';
constexpr inline std::string_view MSG_ERROR{ "ERROR" };
constexpr inline char VALUE_END_CHR = '!';
constexpr inline char FIELD_END_CHR = '&';
constexpr inline char CONTAINER_ELEM_END = '^';

using StringIt = std::string_view::iterator;

std::string_view dataSubstr(StringIt& begin, StringIt end, char dataBreak);

template<typename T>
concept Aggregate = std::is_aggregate_v<std::remove_cvref_t<T>>;

template<std::integral Integral>
void parseValueFromString(std::string_view dataStr, Integral& integral) {
	std::println("Parsing number from {}", dataStr);
	integral = boost::lexical_cast<Integral>(dataStr);
}

template<ranges::viewable_range Range>
void parseValueFromString(std::string_view str, Range& range) {
	std::println("Parsing container: {}", str);

	auto begin = str.begin();
	auto size = boost::lexical_cast<size_t>(dataSubstr(begin, str.end(), CONTAINER_ELEM_END));

	auto parseElem = [&]() {
		auto elemStr = dataSubstr(begin, str.end(), CONTAINER_ELEM_END);
		//std::println("Parsing element {}", elemStr);
		typename Range::value_type elem{};
		parseValueFromString(elemStr, elem);
		range.insert(range.end(), std::move(elem));
	};
	
	for (size_t i = 0; i < size; i++) {
		parseElem();
	}
}

template<typename T>
concept Enum = std::is_enum_v<std::remove_cvref_t<T>>;

template<Enum Enm>
void parseValueFromString(std::string_view str, Enm& enm) {
	using UnderylingType = std::underlying_type_t<Enm>;
	UnderylingType num{};
	parseValueFromString(str, num);
	enm = static_cast<Enm>(num);
}

template<Aggregate T>
void parseValueFromString(std::string_view str, T& aggr) {
	auto begin = str.begin();
	auto parse = [&](auto& field) {
		auto dataStr = dataSubstr(begin, str.end(), FIELD_END_CHR);
		//std::println("Parsing this field string: {}", dataStr);
		parseValueFromString(dataStr, field);
	};
	pfr::for_each_field(aggr, parse);
}

void readMsg(std::string_view) = delete; //you have to pass in at least one argument

template<typename... Args>
void readMsg(std::string_view str, Args&... args) {
	std::println("Parsing: {}", str);
	auto begin = str.begin();
	auto parse = [&](auto& arg) {
		auto dataStr = dataSubstr(begin, str.end(), VALUE_END_CHR);
		parseValueFromString(dataStr, arg);
	};
	((parse(args)), ...);
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
	ret.append(toString(ranges::size(range)) + CONTAINER_ELEM_END);
	for (const auto& elem : range) {
		ret.append(toString(elem) + CONTAINER_ELEM_END);
	}
	return ret;
}

template<Enum Enm>
std::string toString(Enm enm) {
	using UnderlyingType = std::underlying_type_t<Enm>;
	return toString(static_cast<UnderlyingType>(enm));
}

template<Aggregate Aggr>
std::string toString(const Aggr& aggr) {
	std::string buff;
	pfr::for_each_field(aggr, [&](const auto& field) {
		buff.append(toString(field) + FIELD_END_CHR);
	});
	return buff;
}

template<typename... Args>
void writeMsg(std::string& buff, Args&&... args) {
	((buff.append(toString(std::forward<Args>(args)) + VALUE_END_CHR)), ...);
	buff.push_back(MSG_END_DELIM);
}

class InputBuffer {
private:
	std::reference_wrapper<tcp::socket> m_sock;
	std::string m_data;
	std::string::iterator m_begin = m_data.end();
public:
	InputBuffer(tcp::socket& sock);
	std::string_view read();
	asio::awaitable<std::string_view> asyncRead();
};

class OutputBuffer {
private:
	std::string m_data;
public:
	template<typename... Args>
	void send(tcp::socket& sock, Args&&... args) {
		m_data.clear();
		writeMsg(m_data, std::forward<Args>(args)...);
		asio::write(sock, asio::buffer(m_data));
		std::println("Sent {}", m_data);
	}

	template<typename... Args>
	asio::awaitable<void> asyncSend(tcp::socket& sock, Args&&... args) {
		m_data.clear();
		writeMsg(m_data, std::forward<Args>(args)...);
		co_await asio::async_write(sock, asio::buffer(m_data), asio::use_awaitable);
	}
};

class Socket {
private:
	tcp::socket m_sock;
	InputBuffer m_inbox;
	OutputBuffer m_sendBuff;
public:
	template<typename Executor>
	explicit Socket(Executor& exec) : m_sock{ exec }, m_inbox{ m_sock }
	{
	}
	explicit Socket(tcp::socket&& sock);
	
	asio::any_io_executor getExecutor();

	bool connect(const tcp::endpoint& endpoint, sys::error_code& ec);
	asio::awaitable<void> asyncConnect(const tcp::endpoint& endpoint, sys::error_code& ec);

	void disconnect();

	template<typename... Args>
	void read(Args&... args) {
		auto msg = m_inbox.read();
		std::println("Parsing {}", msg);
		readMsg(msg, args...);
	}

	template<typename... Args>
	asio::awaitable<void> asyncRead(Args&... args) {
		auto msg = co_await m_inbox.asyncRead();
		readMsg(msg, args...);
	}
	
	template<typename... Args>
	void send(Args&&... args) {
		m_sendBuff.send(m_sock, args...);
	}

	template<typename... Args>
	asio::awaitable<void> asyncSend(Args&&... args) {
		co_await m_sendBuff.asyncSend(m_sock, std::forward<Args>(args)...);
	}
};