#ifndef BLANK_NET_PACKET_HPP_
#define BLANK_NET_PACKET_HPP_

#include <cstdint>
#include <string>


namespace blank {

struct Packet {

	static constexpr std::uint32_t TAG = 0xFB1AB1AF;

	enum Type {
		PING = 0,
		LOGIN = 1,
	};

	struct TControl {
		std::uint16_t seq;
		std::uint16_t ack;
		std::uint32_t hist;
	};

	struct Header {
		std::uint32_t tag;
		TControl ctrl;
		std::uint8_t type;
	} header;

	std::uint8_t payload[500 - sizeof(Header)];


	void Tag() noexcept;

	std::size_t Ping() noexcept;
	std::size_t Login(const std::string &name) noexcept;

};

}

#endif
