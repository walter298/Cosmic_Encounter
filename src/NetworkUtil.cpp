#include "NetworkUtil.h"

std::string nextDatum(StringIt& begin, StringIt end) {
	auto datumBreak = std::ranges::find(begin, end, DATUM_BREAK_CHR);
	std::string ret{ begin, end };
	begin = std::next(datumBreak);
	return ret;
}

Socket::Socket(tcp::socket&& sock)
	: m_sock{ std::move(sock) }
{
}

asio::any_io_executor Socket::getExecutor() {
	return m_sock.get_executor();
}

void Socket::setReconnection(ReconnCB&& cb) {
	m_reconnect = std::move(cb);
}
void Socket::setAsyncReconnection(ReconnCB&& cb) {
	m_asyncReconnect = std::move(cb);
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

void Socket::disconnect() {
	m_sock.close();
}