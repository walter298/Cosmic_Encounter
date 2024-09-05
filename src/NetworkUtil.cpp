#include "NetworkUtil.h"

#include <functional>

asio::awaitable<std::optional<Socket::Error>> Socket::asyncConnect(const tcp::endpoint& endpoint) noexcept {
	sys::error_code ec;
	co_await m_sock.async_connect(endpoint, asio::redirect_error(asio::use_awaitable, ec));
	co_return checkError(ec, std::source_location::current());
}

Socket::Socket(tcp::socket&& sock) noexcept : m_sock{ std::move(sock) }
{
}

asio::any_io_executor Socket::getExecutor() noexcept {
	return m_sock.get_executor();
}

std::optional<Socket::Error> Socket::connect(const tcp::endpoint& endpoint) noexcept {
	sys::error_code ec;
	m_sock.connect(endpoint, ec);
	return checkError(ec, std::source_location::current());
}

std::optional<Socket::Error> Socket::checkError(sys::error_code error, std::source_location current) noexcept {
	if (error) {
		std::println("Socket Error: {} on line {} in {}", error.message(), current.line(), current.file_name());
		assert(false);
		return Error{ error, current };
	}
	return std::nullopt;
}

std::optional<Socket::Error> Socket::checkError(zpp::bits::errc error, std::source_location current) noexcept {
	if (zpp::bits::failure(error)) {
		Error ret{ std::make_error_code(error), current };
		std::println("Socket Error: {} in {}", ret.ec.message(), current.function_name());
		assert(false);
		return ret;
	}
	return std::nullopt;
}

void Socket::writePayload(size_t payload) noexcept {
	assert(payload < MAX_MESSAGE_SIZE - MAX_PAYLOAD_DIGITS);

	std::span payloadHeader{ m_outputBuff.begin(), MAX_PAYLOAD_DIGITS };

	//write the payload digits into the buffer
	auto [ptr, ec] = std::to_chars(payloadHeader.data(), payloadHeader.data() + MAX_PAYLOAD_DIGITS, payload);
	if (ec != std::errc{}) {
		std::println("{}", std::make_error_code(ec).message());
		std::abort();
	}

	//fill in empty digits with an 'x'
	int digitCount = 0;
	while (payload > 0) {
		payload /= 10;
		digitCount++;
	}
	std::ranges::fill(payloadHeader.begin() + digitCount, payloadHeader.begin() + MAX_PAYLOAD_DIGITS, 'x');
}

size_t Socket::getPayload() {
	size_t end = 0;
	while (std::isdigit(m_inputBuff[end]) && end < 3) {
		end++;
	}
	assert(end > 0);

	assert(!m_inputBuff.empty());
	size_t payload = 0;
	auto [ptr, ec] = std::from_chars(m_inputBuff.data(), m_inputBuff.data() + end, payload);
	if (ec != std::errc{}) {
		std::println("Error parsing payload: {}", std::make_error_code(ec).message());
		std::abort();
	}
	return payload;
}
