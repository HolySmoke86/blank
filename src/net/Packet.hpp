#ifndef BLANK_NET_PACKET_HPP_
#define BLANK_NET_PACKET_HPP_

#include <cstdint>
#include <ostream>
#include <string>
#include <SDL_net.h>
#include <glm/glm.hpp>


namespace blank {

class Block;
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
		std::uint16_t AckEnd() const noexcept { return ack - std::uint16_t(33); }
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

		/// WARNING: do not use these if the data doesn not
		///          point into a real packet's payload
		const Packet &GetPacket() const noexcept {
			return *reinterpret_cast<const Packet *>(data - sizeof(Header));
		}
		const Header &GetHeader() const noexcept {
			return GetPacket().header;
		}
		std::uint16_t Seq() const noexcept {
			return GetHeader().ctrl.seq;
		}

		template<class T>
		void Write(const T &, size_t off) noexcept;
		template<class T>
		void Read(T &, size_t off) const noexcept;

		void Write(const glm::quat &, size_t off) noexcept;
		void Read(glm::quat &, size_t off) const noexcept;
		void Write(const EntityState &, size_t off) noexcept;
		void Read(EntityState &, size_t off) const noexcept;
		void Write(const EntityState &, const glm::ivec3 &, size_t off) noexcept;
		void Read(EntityState &, const glm::ivec3 &, size_t off) const noexcept;

		void WriteString(const std::string &src, std::size_t off, std::size_t maxlen) noexcept;
		void ReadString(std::string &dst, std::size_t off, std::size_t maxlen) const noexcept;

		void WritePackB(const glm::ivec3 &, size_t off) noexcept;
		void ReadPackB(glm::ivec3 &, size_t off) const noexcept;

		void WritePackN(float, size_t off) noexcept;
		void ReadPackN(float &, size_t off) const noexcept;
		void WritePackN(const glm::vec3 &, size_t off) noexcept;
		void ReadPackN(glm::vec3 &, size_t off) const noexcept;

		void WritePackU(float, size_t off) noexcept;
		void ReadPackU(float &, size_t off) const noexcept;
		void WritePackU(const glm::vec3 &, size_t off) noexcept;
		void ReadPackU(glm::vec3 &, size_t off) const noexcept;
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
		static constexpr std::size_t MAX_LEN = 78;

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
		static constexpr std::size_t MAX_LEN = 50;

		void WritePredictedState(const EntityState &) noexcept;
		void ReadPredictedState(EntityState &) const noexcept;
		void WriteMovement(const glm::vec3 &) noexcept;
		void ReadMovement(glm::vec3 &) const noexcept;
		void WriteActions(std::uint8_t) noexcept;
		void ReadActions(std::uint8_t &) const noexcept;
		void WriteSlot(std::uint8_t) noexcept;
		void ReadSlot(std::uint8_t &) const noexcept;
	};

	struct SpawnEntity : public Payload {
		static constexpr std::uint8_t TYPE = 5;
		static constexpr std::size_t MAX_LEN = 110;

		void WriteEntity(const Entity &) noexcept;
		void ReadEntityID(std::uint32_t &) const noexcept;
		void ReadModelID(std::uint32_t &) const noexcept;
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
		static constexpr std::size_t MAX_LEN = 460;

		static constexpr std::uint32_t MAX_ENTITIES = 12;
		static constexpr std::size_t GetSize(std::uint32_t num) noexcept {
			return 16 + (num * 37);
		}

		void WriteEntityCount(std::uint32_t) noexcept;
		void ReadEntityCount(std::uint32_t &) const noexcept;
		void WriteChunkBase(const glm::ivec3 &) noexcept;
		void ReadChunkBase(glm::ivec3 &) const noexcept;

		void WriteEntity(const Entity &, const glm::ivec3 &, std::uint32_t) noexcept;
		void ReadEntityID(std::uint32_t &, std::uint32_t) const noexcept;
		void ReadEntityState(EntityState &, const glm::ivec3 &, std::uint32_t) const noexcept;
	};

	struct PlayerCorrection : public Payload {
		static constexpr std::uint8_t TYPE = 8;
		static constexpr std::size_t MAX_LEN = 44;

		void WritePacketSeq(std::uint16_t) noexcept;
		void ReadPacketSeq(std::uint16_t &) const noexcept;
		void WritePlayer(const Entity &) noexcept;
		void ReadPlayerState(EntityState &) const noexcept;
	};

	struct ChunkBegin : public Payload {
		static constexpr std::uint8_t TYPE = 9;
		static constexpr std::size_t MAX_LEN = 24;

		void WriteTransmissionId(std::uint32_t) noexcept;
		void ReadTransmissionId(std::uint32_t &) const noexcept;
		void WriteFlags(std::uint32_t) noexcept;
		void ReadFlags(std::uint32_t &) const noexcept;
		void WriteChunkCoords(const glm::ivec3 &) noexcept;
		void ReadChunkCoords(glm::ivec3 &) const noexcept;
		void WriteDataSize(std::uint32_t) noexcept;
		void ReadDataSize(std::uint32_t &) const noexcept;
	};

	struct ChunkData : public Payload {
		static constexpr std::uint8_t TYPE = 10;
		static constexpr std::size_t MAX_LEN = MAX_PAYLOAD_LEN;
		static constexpr std::size_t MAX_DATA_LEN = MAX_LEN - 12;

		static constexpr std::size_t GetSize(std::size_t data_len) noexcept {
			return data_len + 12;
		}

		void WriteTransmissionId(std::uint32_t) noexcept;
		void ReadTransmissionId(std::uint32_t &) const noexcept;
		void WriteDataOffset(std::uint32_t) noexcept;
		void ReadDataOffset(std::uint32_t &) const noexcept;
		void WriteDataSize(std::uint32_t) noexcept;
		void ReadDataSize(std::uint32_t &) const noexcept;
		void WriteData(const std::uint8_t *, std::size_t len) noexcept;
		void ReadData(std::uint8_t *, std::size_t maxlen) const noexcept;
	};

	struct BlockUpdate : public Payload {
		static constexpr std::uint8_t TYPE = 11;
		static constexpr std::size_t MAX_LEN = 484;

		static constexpr std::uint32_t MAX_BLOCKS = 78;
		static constexpr std::size_t GetSize(std::uint32_t num) noexcept {
			return 16 + (num * 6);
		}

		void WriteChunkCoords(const glm::ivec3 &) noexcept;
		void ReadChunkCoords(glm::ivec3 &) const noexcept;
		void WriteBlockCount(std::uint32_t) noexcept;
		void ReadBlockCount(std::uint32_t &) const noexcept;

		void WriteIndex(std::uint16_t, std::uint32_t) noexcept;
		void ReadIndex(std::uint16_t &, std::uint32_t) const noexcept;
		void WriteBlock(const Block &, std::uint32_t) noexcept;
		void ReadBlock(Block &, std::uint32_t) const noexcept;
	};

	struct Message : public Payload {
		static constexpr std::uint8_t TYPE = 12;
		static constexpr std::size_t MAX_LEN = 455;

		static constexpr std::size_t MAX_MESSAGE_LEN = 450;
		static std::size_t GetSize(const std::string &msg) noexcept {
			return 5 + std::min(msg.size() + 1, MAX_MESSAGE_LEN);
		}

		void WriteType(std::uint8_t) noexcept;
		void ReadType(std::uint8_t &) const noexcept;
		void WriteReferral(std::uint32_t) noexcept;
		void ReadReferral(std::uint32_t &) const noexcept;
		void WriteMessage(const std::string &) noexcept;
		void ReadMessage(std::string &) const noexcept;
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
