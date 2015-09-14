#ifndef BLANK_NET_PACKET_HPP_
#define BLANK_NET_PACKET_HPP_

#include <cstdint>
#include <ostream>
#include <string>
#include <SDL_net.h>


namespace blank {

class Entity;
class EntityState;

struct Packet {

	static constexpr std::uint32_t TAG = 0xFB1AB1AF;

	static const char *Type2String(std::uint8_t) noexcept;

	struct TControl {
		std::uint16_t seq;
		std::uint16_t ack;
		std::uint32_t hist;

		// true if this contains an ack for given (remote) seq
		bool Acks(std::uint16_t) const noexcept;
		std::uint16_t AckBegin() const noexcept { return ack; }
		std::uint16_t AckEnd() const noexcept { return ack + std::uint16_t(33); }
	};

	struct Header {
		std::uint32_t tag;
		TControl ctrl;
		std::uint8_t type;
		std::uint8_t reserved1;
		std::uint8_t reserved2;
		std::uint8_t reserved3;
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

		std::uint16_t Seq() const noexcept {
			return reinterpret_cast<const Packet *>(data - sizeof(Header))->header.ctrl.seq;
		}

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
		void ReadPlayerID(std::uint32_t &) const noexcept;
		void ReadPlayerState(EntityState &) const noexcept;
		void WriteWorldName(const std::string &) noexcept;
		void ReadWorldName(std::string &) const noexcept;
	};

	struct Part : public Payload {
		static constexpr std::uint8_t TYPE = 3;
		static constexpr std::size_t MAX_LEN = 0;
	};

	struct PlayerUpdate : public Payload {
		static constexpr std::uint8_t TYPE = 4;
		static constexpr std::size_t MAX_LEN = 64;

		void WritePlayer(const Entity &) noexcept;
		void ReadPlayerState(EntityState &) const noexcept;
	};

	struct SpawnEntity : public Payload {
		static constexpr std::uint8_t TYPE = 5;
		static constexpr std::size_t MAX_LEN = 132;

		void WriteEntity(const Entity &) noexcept;
		void ReadEntityID(std::uint32_t &) const noexcept;
		void ReadSkeletonID(std::uint32_t &) const noexcept;
		void ReadEntity(Entity &) const noexcept;
	};

	struct DespawnEntity : public Payload {
		static constexpr std::uint8_t TYPE = 6;
		static constexpr std::size_t MAX_LEN = 4;

		void WriteEntityID(std::uint32_t) noexcept;
		void ReadEntityID(std::uint32_t &) const noexcept;
	};

	struct EntityUpdate : public Payload {
		static constexpr std::uint8_t TYPE = 7;
		static constexpr std::size_t MAX_LEN = 452;

		static constexpr std::uint32_t MAX_ENTITIES = 7;
		static constexpr std::size_t GetSize(std::uint32_t num) noexcept {
			return 4 + (num * 64);
		}

		void WriteEntityCount(std::uint32_t) noexcept;
		void ReadEntityCount(std::uint32_t &) const noexcept;

		void WriteEntity(const Entity &, std::uint32_t) noexcept;
		void ReadEntityID(std::uint32_t &, std::uint32_t) const noexcept;
		void ReadEntityState(EntityState &, std::uint32_t) const noexcept;
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

		udp_pack.len = sizeof(Header) + PayloadType::MAX_LEN;

		PayloadType result;
		result.length = PayloadType::MAX_LEN;
		result.data = pack.payload;
		return result;
	}

};

}

#endif
