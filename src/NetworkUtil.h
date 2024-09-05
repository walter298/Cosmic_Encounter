#pragma once

#include <charconv>        //integer parsing
#include <expected>        //serialization buffer/error
#include <print>           //error printing
#include <source_location> //socket error reporting
#include <string_view>     //Socket::DELIM
#include <vector>          //NonContainerRange serialization overload

#include <SDKDDKVer.h> //windows version macros for boost.asio

#define BOOST_ASIO_DISABLE_HANDLER_TYPE_REQUIREMENTS

#include <boost/asio.hpp> //networking

#include <magic_enum.hpp> //socket error reporting
#include <zpp_bits.h> //serialization and deserialization

#include "novalis/data_util/BasicConcepts.h" //NonContainerRange
#include "novalis/data_util/DataStructures.h" //ScopeExit

#include "SocketMessage.h"

//boost namespace aliases
namespace asio = boost::asio;
namespace ip = asio::ip;
using ip::tcp;
namespace sys = boost::system;

template<typename Serializer, nv::NonContainerRange Range>
auto serialize(Serializer& serializer, Range&& range) {
	std::vector<std::ranges::range_value_t<Range>> vec;
	vec.append_range(std::forward<Range>(range));
	return serializer(std::move(vec));
}

#define COSMIC_CHECK_OPTIONAL_NETWORK_ERROR(error) \
	{ \
		auto ec = checkError(error, std::source_location::current()); \
		if (ec) { \
			return ec; \
		} \
	}
#define COSMIC_CHECK_OPTIONAL_CORO_NETWORK_ERROR(error) \
	{ \
		auto ec = checkError(error, std::source_location::current()); \
		if (ec) { \
			co_return ec; \
		} \
	}
#define COSMIC_CHECK_UNEXPECTED_NETWORK_ERROR(error) \
	{ \
		auto ec = checkError(error, std::source_location::current()); \
		if (ec) { \
			return std::unexpected{ *ec }; \
		} \
	}
 
class Socket {
private:
	static constexpr size_t MAX_MESSAGE_SIZE = 512;
	static constexpr size_t MAX_PAYLOAD_DIGITS = 3;

	using Buffer = std::array<char, MAX_MESSAGE_SIZE>;

	tcp::socket m_sock;

	Buffer m_outputBuff;
	Buffer m_inputBuff;
public:
	struct Error {
		sys::error_code ec;
		std::source_location loc;
	};

	template<typename Executor>
	explicit Socket(Executor& exec) noexcept : m_sock{ exec }
	{
	}
	explicit Socket(tcp::socket&& sock) noexcept;

	asio::any_io_executor getExecutor() noexcept;

	std::optional<Error> connect(const tcp::endpoint& endpoint) noexcept;
	asio::awaitable<std::optional<Error>> asyncConnect(const tcp::endpoint& endpoint) noexcept;

	std::optional<Error> checkError(sys::error_code error, std::source_location current) noexcept;
	std::optional<Error> checkError(zpp::bits::errc error, std::source_location current) noexcept;
private:
	using SerializedBuffer = std::expected<decltype(asio::buffer(m_outputBuff.data(), 0)), Error>;

	void writePayload(size_t payload) noexcept;

	template<typename... Args>
	SerializedBuffer serializeImpl(SocketHeader header, Args&&... args) noexcept {
		std::span body{ m_outputBuff.begin() + MAX_PAYLOAD_DIGITS, m_outputBuff.size() - MAX_PAYLOAD_DIGITS};
		zpp::bits::out bodySerializer{ body };

		//serialize header and args, leaving room for payload at beginning
		COSMIC_CHECK_UNEXPECTED_NETWORK_ERROR(bodySerializer(header, std::forward<Args>(args)...));

		auto payload = bodySerializer.processed_data().size();
		writePayload(payload);

		auto totalMsgLen = payload + MAX_PAYLOAD_DIGITS;
		assert(totalMsgLen <= MAX_MESSAGE_SIZE);

		return asio::buffer(m_outputBuff, totalMsgLen);
	}
public:
	template<typename... Args>
	std::optional<Error> send(SocketHeader expectedHeader, Args&&... args) noexcept {
		//serialize and do error checking
		auto serializedDataBuff = serializeImpl(expectedHeader, std::forward<Args>(args)...);
		if (!serializedDataBuff) {
			return serializedDataBuff.error();
		}

		sys::error_code writeError;
		auto bytesWritten = asio::write(m_sock, serializedDataBuff.value(), writeError);
		COSMIC_CHECK_OPTIONAL_NETWORK_ERROR(writeError);

		return std::nullopt;
	}
	template<typename... Args>
	asio::awaitable<std::optional<Error>> asyncSend(SocketHeader msg, Args&&... args) noexcept {
		assert(m_sock.is_open());
		auto serializedDataBuff = serializeImpl(msg, std::forward<Args>(args)...);
		if (!serializedDataBuff) {
			co_return serializedDataBuff.error();
		}

		//try to send bytes over the network
		sys::error_code writeError;
		co_await asio::async_write(m_sock, serializedDataBuff.value(), asio::redirect_error(asio::use_awaitable, writeError));
		COSMIC_CHECK_OPTIONAL_CORO_NETWORK_ERROR(writeError);

		co_return std::nullopt;
	}
private:
	template<typename... Args>
	std::optional<Error> deserializeImpl(SocketHeader expectedHeader, Args&... args) noexcept {
		std::span body{ m_inputBuff.begin() + MAX_PAYLOAD_DIGITS, m_inputBuff.end() };
		zpp::bits::in deserializer{ body };

		//validate that the correct message is being read
		auto temp = expectedHeader;
		COSMIC_CHECK_OPTIONAL_NETWORK_ERROR(deserializer(temp));
		if (temp != expectedHeader) {
			std::println("Error: Socket expected to parse {} but {} was sent", 
				magic_enum::enum_name(expectedHeader), magic_enum::enum_name(temp));
			std::abort();
		}

		//deserialize everything else
		COSMIC_CHECK_OPTIONAL_NETWORK_ERROR(deserializer(args...));
		
		return std::nullopt;
	}

	size_t getPayload();

	auto getBodyBuffer() {
		return asio::buffer(m_inputBuff.data() + MAX_PAYLOAD_DIGITS, m_inputBuff.size() - MAX_PAYLOAD_DIGITS);
	}
public:
	template<typename... Args>
	std::optional<Error> read(SocketHeader expectedHeader, Args&... args) noexcept {
		assert(m_sock.is_open());

		sys::error_code readError;

		//read in the payload size
		asio::read(m_sock, asio::buffer(m_inputBuff), asio::transfer_exactly(MAX_PAYLOAD_DIGITS), readError);
		COSMIC_CHECK_OPTIONAL_NETWORK_ERROR(readError);
		
		//read the body, whose length is exactly "payload" bytes
		asio::read(m_sock, getBodyBuffer(), asio::transfer_exactly(getPayload()), readError);

		COSMIC_CHECK_OPTIONAL_NETWORK_ERROR(readError);

		//deserialize and do error checking
		return deserializeImpl(expectedHeader, args...);
	}
	template<typename... Args>
	asio::awaitable<std::optional<Error>> asyncRead(SocketHeader expectedHeader, Args&... args) noexcept {
		assert(m_sock.is_open());

		sys::error_code readError;

		//read in the payload size
		co_await asio::async_read(m_sock, asio::buffer(m_inputBuff), asio::transfer_exactly(MAX_PAYLOAD_DIGITS),
			asio::redirect_error(asio::use_awaitable, readError));
		COSMIC_CHECK_OPTIONAL_CORO_NETWORK_ERROR(readError);

		//read the body, whose length is exactly "payload" bytes
		co_await asio::async_read(m_sock, getBodyBuffer(), asio::transfer_exactly(getPayload()), 
			asio::redirect_error(asio::use_awaitable, readError));
		COSMIC_CHECK_OPTIONAL_CORO_NETWORK_ERROR(readError);

		//deserialize and do error checking
		co_return deserializeImpl(expectedHeader, args...);
	}
};