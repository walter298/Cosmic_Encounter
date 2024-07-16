#include "NetworkUtil.h"

std::string_view dataSubstr(StringIt& begin, StringIt end, char dataBreak) {
	auto dataBreakIt = ranges::find(begin, end, dataBreak);
	std::string_view ret{ begin, dataBreakIt };
	begin = std::next(dataBreakIt);
	return ret;
}

Socket::Socket(tcp::socket&& sock)
	: m_sock{ std::move(sock) }
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