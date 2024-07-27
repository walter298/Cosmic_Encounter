#include "NetworkUtil.h"

#include <functional> //boyer moore search

std::string_view dataSubstr(StringIt& begin, StringIt end, char dataBreak) {
	auto dataBreakIt = ranges::find(begin, end, dataBreak);
	std::string_view ret{ begin, dataBreakIt };
	begin = std::next(dataBreakIt);
	return ret;
}

Socket::Socket(tcp::socket&& sock)
	: m_sock{ std::move(sock) }, m_inbox{ m_sock }
{
}

asio::any_io_executor Socket::getExecutor() {
	return m_sock.get_executor();
}

bool Socket::connect(const tcp::endpoint& endpoint, sys::error_code& ec) {
	tcp::resolver res{ m_sock.get_executor() };
	auto result = res.resolve(endpoint, ec);
	if (ec) { return false; }
	asio::connect(m_sock, result, ec);
	if (ec) { 
		return true; 
	} else {
		return false;
	}
}

asio::awaitable<void> Socket::asyncConnect(const tcp::endpoint& endpoint, sys::error_code& ec) {
	co_await m_sock.async_connect(endpoint, asio::redirect_error(asio::use_awaitable, ec));
}

void Socket::disconnect() {
	m_sock.close();
}

std::string_view InputBuffer::parseLatestMessage() {
	//parse new message
	auto msgEndBeginIt = ranges::find(ranges::subrange(m_begin, m_data.end()), MSG_END_DELIM);
	std::string_view ret{ m_begin, msgEndBeginIt };

	m_begin = msgEndBeginIt + 1; //update data begin

	return ret;
}

InputBuffer::InputBuffer(tcp::socket& sock)
	: m_sock{ sock }
{
}

std::string_view InputBuffer::read() {
	if (m_begin == m_data.end()) {
		m_data.clear();
		asio::read_until(m_sock.get(), asio::dynamic_buffer(m_data), MSG_END_DELIM);
		m_begin = m_data.begin();
	}
	return parseLatestMessage();
}

asio::awaitable<std::string_view> InputBuffer::asyncRead() {
	if (m_begin == m_data.end()) {
		m_data.clear();
		co_await asio::async_read_until(m_sock.get(), asio::dynamic_buffer(m_data), MSG_END_DELIM, asio::use_awaitable);
		m_begin = m_data.begin();
	}
	co_return parseLatestMessage();
}