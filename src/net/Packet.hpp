#ifndef BLANK_NET_PACKET_HPP_
#define BLANK_NET_PACKET_HPP_

#include <cstdint>
#include <ostream>
#include <string>
#include <SDL_net.h>


namespace blank {

class Entity;

struct Packet {

	static constexpr std::uint32_t TAG = 0xFB1AB1AF;

	static const char *Type2String(std::uint8_t) noexcept;

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

	static constexpr std::size_t MAX_PAYLOAD_LEN = 500 - sizeof(Header);

	std::uint8_t payload[MAX_PAYLOAD_LEN];


	void Tag() noexcept { header.tag = TAG; }

	void Type(std::uint8_t t) noexcept { header.type = t; }
	std::uint8_t Type() const noexcept { return header.type; }
	const char *TypeString() const noexcept { return Type2String(Type()); }


	struct Payload {
		std::size_t length;
		std::uint8_t *data;

		template<class T>
		void Write(const T &, size_t off) noexcept;
		template<class T>
		void Read(T &, size_t off) const noexcept;

		void WriteString(const std::string &src, std::size_t off, std::size_t maxlen) noexcept;
		void ReadString(std::string &dst, std::size_t off, std::size_t maxlen) const noexcept;
	};

	struct Ping : public Payload {
		static constexpr std::uint8_t TYPE = 0;
		static constexpr std::size_t MAX_LEN = 0;
	};

	struct Login : public Payload {
		static constexpr std::uint8_t TYPE = 1;
		static constexpr std::size_t MAX_LEN = 32;

		void WritePlayerName(const std::string &) noexcept;
		void ReadPlayerName(std::string &) const noexcept;
	};

	struct Join : public Payload {
		static constexpr std::uint8_t TYPE = 2;
		static constexpr std::size_t MAX_LEN = 100;

		void WritePlayer(const Entity &) noexcept;
		void ReadPlayer(Entity &) const noexcept;
		void WriteWorldName(const std::string &) noexcept;
		void ReadWorldName(std::string &) const noexcept;
	};

	struct Part : public Payload {
		static constexpr std::uint8_t TYPE = 3;
		static constexpr std::size_t MAX_LEN = 0;
	};


	template<class PayloadType>
	PayloadType As() {
		PayloadType result;
		result.length = PayloadType::MAX_LEN;
		result.data = &payload[0];
		return result;
	}

	template<class PayloadType>
	static PayloadType As(const UDPpacket &pack) {
		PayloadType result;
		result.length = std::min(pack.len - sizeof(Header), PayloadType::MAX_LEN);
		result.data = pack.data + sizeof(Header);
		return result;
	}

	template<class PayloadType>
	static PayloadType Make(UDPpacket &udp_pack) {
		Packet &pack = *reinterpret_cast<Packet *>(udp_pack.data);
		pack.Tag();
		pack.Type(PayloadType::TYPE);

		udp_pack.len = sizeof(Header) + PayloadType::TYPE;

		PayloadType result;
		result.length = PayloadType::MAX_LEN;
		result.data = pack.payload;
		return result;
	}

};

}

#endif
