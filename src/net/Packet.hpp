#ifndef BLANK_NET_PACKET_HPP_
#define BLANK_NET_PACKET_HPP_

#include <cstdint>
#include <ostream>
#include <string>


namespace blank {

class Entity;

struct Packet {

	static constexpr std::uint32_t TAG = 0xFB1AB1AF;

	enum Type {
		PING = 0,
		LOGIN = 1,
		JOIN = 2,
		PART = 3,
	};

	static const char *Type2String(Type) noexcept;

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


	Type GetType() const noexcept { return Type(header.type); }

	void Tag() noexcept;

	std::size_t MakePing() noexcept;
	std::size_t MakeLogin(const std::string &name) noexcept;
	std::size_t MakeJoin(const Entity &player, const std::string &world_name) noexcept;
	std::size_t MakePart() noexcept;

};

inline std::ostream &operator <<(std::ostream &out, Packet::Type t) {
	return out << Packet::Type2String(t);
}

}

#endif
