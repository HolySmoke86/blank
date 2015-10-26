#include "Connection.hpp"
#include "ConnectionHandler.hpp"
#include "io.hpp"
#include "Packet.hpp"

#include "../app/init.hpp"
#include "../model/Model.hpp"
#include "../world/Entity.hpp"
#include "../world/EntityState.hpp"

#include <cstring>

using namespace std;


namespace blank {

constexpr size_t Packet::Ping::MAX_LEN;
constexpr size_t Packet::Login::MAX_LEN;
constexpr size_t Packet::Join::MAX_LEN;
constexpr size_t Packet::Part::MAX_LEN;
constexpr size_t Packet::PlayerUpdate::MAX_LEN;
constexpr size_t Packet::SpawnEntity::MAX_LEN;
constexpr size_t Packet::DespawnEntity::MAX_LEN;
constexpr size_t Packet::EntityUpdate::MAX_LEN;
constexpr size_t Packet::PlayerCorrection::MAX_LEN;
constexpr size_t Packet::ChunkBegin::MAX_LEN;
constexpr size_t Packet::ChunkData::MAX_LEN;
constexpr size_t Packet::BlockUpdate::MAX_LEN;
constexpr size_t Packet::Message::MAX_LEN;
constexpr size_t Packet::Message::MAX_MESSAGE_LEN;

Connection::Connection(const IPaddress &addr)
: handler(nullptr)
, addr(addr)
, send_timer(500)
, recv_timer(10000)
, ctrl_out{ 0, 0xFFFF, 0xFFFFFFFF }
, ctrl_in{ 0, 0xFFFF, 0xFFFFFFFF }
, closed(false) {
	send_timer.Start();
	recv_timer.Start();
}

bool Connection::Matches(const IPaddress &remote) const noexcept {
	return memcmp(&addr, &remote, sizeof(IPaddress)) == 0;
}

void Connection::FlagSend() noexcept {
	send_timer.Reset();
}

void Connection::FlagRecv() noexcept {
	recv_timer.Reset();
}

bool Connection::ShouldPing() const noexcept {
	return !closed && send_timer.HitOnce();
}

bool Connection::TimedOut() const noexcept {
	return recv_timer.HitOnce();
}

void Connection::Update(int dt) {
	send_timer.Update(dt);
	recv_timer.Update(dt);
	if (TimedOut()) {
		Close();
		if (HasHandler()) {
			Handler().OnTimeout();
		}
	}
}


uint16_t Connection::Send(UDPpacket &udp_pack, UDPsocket sock) {
	Packet &pack = *reinterpret_cast<Packet *>(udp_pack.data);
	pack.header.ctrl = ctrl_out;
	uint16_t seq = ctrl_out.seq++;

	udp_pack.address = addr;
	if (SDLNet_UDP_Send(sock, -1, &udp_pack) == 0) {
		throw NetError("SDLNet_UDP_Send");
	}

	if (HasHandler()) {
		Handler().PacketSent(seq);
	}

	FlagSend();
	return seq;
}

void Connection::Received(const UDPpacket &udp_pack) {
	Packet &pack = *reinterpret_cast<Packet *>(udp_pack.data);

	// ack to the remote
	int16_t diff = int16_t(pack.header.ctrl.seq) - int16_t(ctrl_out.ack);
	if (diff > 0) {
		if (diff >= 32) {
			ctrl_out.hist = 0;
		} else {
			ctrl_out.hist <<= diff;
			ctrl_out.hist |= 1 << (diff - 1);
		}
	} else if (diff < 0 && diff >= -32) {
		ctrl_out.hist |= 1 << (-diff - 1);
	}
	ctrl_out.ack = pack.header.ctrl.seq;
	FlagRecv();

	if (!HasHandler()) {
		return;
	}

	Packet::TControl ctrl_new = pack.header.ctrl;
	Handler().Handle(udp_pack);

	if (diff > 0) {
		// if the packet holds more recent information
		// check if remote failed to ack one of our packets
		diff = int16_t(ctrl_new.ack) - int16_t(ctrl_in.ack);
		// should always be true, but you never know…
		if (diff > 0) {
			for (int i = 0; i < diff; ++i) {
				if (i > 32 || (i < 32 && (ctrl_in.hist & (1 << (31 - i))) == 0)) {
					Handler().PacketLost(ctrl_in.ack - 32 + i);
				}
			}
		}
		// check for newly ack'd packets
		for (uint16_t s = ctrl_new.AckBegin(); s != ctrl_new.AckEnd(); --s) {
			if (ctrl_new.Acks(s) && !ctrl_in.Acks(s)) {
				Handler().PacketReceived(s);
			}
		}
		ctrl_in = ctrl_new;
	}
}

bool Packet::TControl::Acks(uint16_t s) const noexcept {
	int16_t diff = int16_t(ack) - int16_t(s);
	if (diff == 0) return true;
	if (diff < 0 || diff > 32) return false;
	return (hist & (1 << (diff - 1))) != 0;
}

uint16_t Connection::SendPing(UDPpacket &udp_pack, UDPsocket sock) {
	Packet::Make<Packet::Ping>(udp_pack);
	return Send(udp_pack, sock);
}


ConnectionHandler::ConnectionHandler()
: packets_lost(0)
, packets_received(0)
, packet_loss(0.0f)
, stamp_cursor(15)
, stamp_last(0)
, rtt(64.0f) {
	Uint32 now = SDL_GetTicks();
	for (Uint32 &s : stamps) {
		s = now;
	}
}

void ConnectionHandler::PacketSent(uint16_t seq) {
	if (!SamplePacket(seq)) {
		return;
	}
	stamp_cursor = (stamp_cursor + 1) % 16;
	stamps[stamp_cursor] = SDL_GetTicks();
	stamp_last = seq;
}

void ConnectionHandler::PacketLost(uint16_t seq) {
	OnPacketLost(seq);
	++packets_lost;
	UpdatePacketLoss();
	UpdateRTT(seq);
}

void ConnectionHandler::PacketReceived(uint16_t seq) {
	OnPacketReceived(seq);
	++packets_received;
	UpdatePacketLoss();
	UpdateRTT(seq);
}

void ConnectionHandler::UpdatePacketLoss() noexcept {
	unsigned int packets_total = packets_lost + packets_received;
	if (packets_total >= 256) {
		packet_loss = float(packets_lost) / float(packets_total);
		packets_lost = 0;
		packets_received = 0;
	}
}

void ConnectionHandler::UpdateRTT(std::uint16_t seq) noexcept {
	if (!SamplePacket(seq)) return;
	int diff = HeadDiff(seq);
	if (diff > 0 || diff < -15) {
		// packet outside observed time frame
		return;
	}
	int cur_rtt = SDL_GetTicks() - stamps[(stamp_cursor + diff + 16) % 16];
	rtt += (cur_rtt - rtt) * 0.1f;
}

bool ConnectionHandler::SamplePacket(std::uint16_t seq) const noexcept {
	// only sample every eighth packet
	return seq % 8 == 0;
}

int ConnectionHandler::HeadDiff(std::uint16_t seq) const noexcept {
	int16_t diff = int16_t(seq) - int16_t(stamp_last);
	return diff / 8;
}


ostream &operator <<(ostream &out, const IPaddress &addr) {
	const unsigned char *host = reinterpret_cast<const unsigned char *>(&addr.host);
	out << int(host[0])
		<< '.' << int(host[1])
		<< '.' << int(host[2])
		<< '.' << int(host[3]);
	if (addr.port) {
		out << ':' << SDLNet_Read16(&addr.port);
	}
	return out;
}


const char *Packet::Type2String(uint8_t t) noexcept {
	switch (t) {
		case Ping::TYPE:
			return "Ping";
		case Login::TYPE:
			return "Login";
		case Join::TYPE:
			return "Join";
		case Part::TYPE:
			return "Part";
		case PlayerUpdate::TYPE:
			return "PlayerUpdate";
		case SpawnEntity::TYPE:
			return "SpawnEntity";
		case DespawnEntity::TYPE:
			return "DespawnEntity";
		case EntityUpdate::TYPE:
			return "EntityUpdate";
		case PlayerCorrection::TYPE:
			return "PlayerCorrection";
		case ChunkBegin::TYPE:
			return "ChunkBegin";
		case ChunkData::TYPE:
			return "ChunkData";
		case BlockUpdate::TYPE:
			return "BlockUpdate";
		case Message::TYPE:
			return "Message";
		default:
			return "Unknown";
	}
}

template<class T>
void Packet::Payload::Write(const T &src, size_t off) noexcept {
	if ((length - off) < sizeof(T)) {
		// dismiss out of bounds write
		return;
	}
	*reinterpret_cast<T *>(&data[off]) = src;
}

template<class T>
void Packet::Payload::Read(T &dst, size_t off) const noexcept {
	if ((length - off) < sizeof(T)) {
		// dismiss out of bounds read
		return;
	}
	dst = *reinterpret_cast<T *>(&data[off]);
}

void Packet::Payload::WriteString(const string &src, size_t off, size_t maxlen) noexcept {
	uint8_t *dst = &data[off];
	size_t len = min(maxlen, length - off);
	if (src.size() < len) {
		memset(dst, '\0', len);
		memcpy(dst, src.c_str(), src.size());
	} else {
		memcpy(dst, src.c_str(), len);
	}
}

void Packet::Payload::ReadString(string &dst, size_t off, size_t maxlen) const noexcept {
	size_t len = min(maxlen, length - off);
	dst.clear();
	dst.reserve(len);
	for (size_t i = 0; i < len && data[off + i] != '\0'; ++i) {
		dst.push_back(data[off + i]);
	}
}

void Packet::Payload::Write(const glm::quat &val, size_t off) noexcept {
	WritePackN(val.w, off);
	WritePackN(val.x, off + 2);
	WritePackN(val.y, off + 4);
	WritePackN(val.z, off + 6);
}

void Packet::Payload::Read(glm::quat &val, size_t off) const noexcept {
	ReadPackN(val.w, off);
	ReadPackN(val.x, off + 2);
	ReadPackN(val.y, off + 4);
	ReadPackN(val.z, off + 6);
	val = normalize(val);
}

void Packet::Payload::Write(const EntityState &state, size_t off) noexcept {
	Write(state.chunk_pos, off);
	WritePackU(state.block_pos * (1.0f / 16.0f), off + 12);
	Write(state.velocity, off + 18);
	Write(state.orient, off + 30);
	WritePackN(state.pitch * PI_0p5_inv, off + 38);
	WritePackN(state.yaw * PI_inv, off + 40);
}

void Packet::Payload::Read(EntityState &state, size_t off) const noexcept {
	Read(state.chunk_pos, off);
	ReadPackU(state.block_pos, off + 12);
	Read(state.velocity, off + 18);
	Read(state.orient, off + 30);
	ReadPackN(state.pitch, off + 38);
	ReadPackN(state.yaw, off + 40);
	state.block_pos *= 16.0f;
	state.pitch *= PI_0p5;
	state.yaw *= PI;
}

void Packet::Payload::Write(const EntityState &state, const glm::ivec3 &base, size_t off) noexcept {
	WritePackB(state.chunk_pos - base, off);
	WritePackU(state.block_pos * (1.0f / 16.0f), off + 3);
	Write(state.velocity, off + 9);
	Write(state.orient, off + 21);
	WritePackN(state.pitch * PI_0p5_inv, off + 29);
	WritePackN(state.yaw * PI_inv, off + 31);
}

void Packet::Payload::Read(EntityState &state, const glm::ivec3 &base, size_t off) const noexcept {
	ReadPackB(state.chunk_pos, off);
	ReadPackU(state.block_pos, off + 3);
	Read(state.velocity, off + 9);
	Read(state.orient, off + 21);
	ReadPackN(state.pitch, off + 29);
	ReadPackN(state.yaw, off + 31);
	state.chunk_pos += base;
	state.block_pos *= 16.0f;
	state.pitch *= PI_0p5;
	state.yaw *= PI;
}

void Packet::Payload::WritePackB(const glm::ivec3 &val, size_t off) noexcept {
	Write(int8_t(val.x), off);
	Write(int8_t(val.y), off + 1);
	Write(int8_t(val.z), off + 2);
}

void Packet::Payload::ReadPackB(glm::ivec3 &val, size_t off) const noexcept {
	int8_t conv = 0;
	Read(conv, off);
	val.x = conv;
	Read(conv, off + 1);
	val.y = conv;
	Read(conv, off + 2);
	val.z = conv;
}

void Packet::Payload::WritePackN(float val, size_t off) noexcept {
	int16_t raw = glm::clamp(glm::round(val * 32767.0f), -32767.0f, 32767.0f);
	Write(raw, off);
}

void Packet::Payload::ReadPackN(float &val, size_t off) const noexcept {
	int16_t raw = 0;
	Read(raw, off);
	val = raw * (1.0f/32767.0f);
}

void Packet::Payload::WritePackN(const glm::vec3 &val, size_t off) noexcept {
	WritePackN(val.x, off);
	WritePackN(val.y, off + 2);
	WritePackN(val.z, off + 4);
}

void Packet::Payload::ReadPackN(glm::vec3 &val, size_t off) const noexcept {
	ReadPackN(val.x, off);
	ReadPackN(val.y, off + 2);
	ReadPackN(val.z, off + 4);
}

void Packet::Payload::WritePackU(float val, size_t off) noexcept {
	uint16_t raw = glm::clamp(glm::round(val * 65535.0f), 0.0f, 65535.0f);
	Write(raw, off);
}

void Packet::Payload::ReadPackU(float &val, size_t off) const noexcept {
	uint16_t raw = 0;
	Read(raw, off);
	val = raw * (1.0f/65535.0f);
}

void Packet::Payload::WritePackU(const glm::vec3 &val, size_t off) noexcept {
	WritePackU(val.x, off);
	WritePackU(val.y, off + 2);
	WritePackU(val.z, off + 4);
}

void Packet::Payload::ReadPackU(glm::vec3 &val, size_t off) const noexcept {
	ReadPackU(val.x, off);
	ReadPackU(val.y, off + 2);
	ReadPackU(val.z, off + 4);
}


void Packet::Login::WritePlayerName(const string &name) noexcept {
	WriteString(name, 0, 32);
}

void Packet::Login::ReadPlayerName(string &name) const noexcept {
	ReadString(name, 0, 32);
}

void Packet::Join::WritePlayer(const Entity &player) noexcept {
	Write(player.ID(), 0);
	Write(player.GetState(), 4);
}

void Packet::Join::ReadPlayerID(uint32_t &id) const noexcept {
	Read(id, 0);
}

void Packet::Join::ReadPlayerState(EntityState &state) const noexcept {
	Read(state, 4);
}

void Packet::Join::WriteWorldName(const string &name) noexcept {
	WriteString(name, 46, 32);
}

void Packet::Join::ReadWorldName(string &name) const noexcept {
	ReadString(name, 46, 32);
}

void Packet::PlayerUpdate::WritePredictedState(const EntityState &state) noexcept {
	Write(state, 0);
}

void Packet::PlayerUpdate::ReadPredictedState(EntityState &state) const noexcept {
	Read(state, 0);
}

void Packet::PlayerUpdate::WriteMovement(const glm::vec3 &mov) noexcept {
	WritePackN(mov, 42);
}

void Packet::PlayerUpdate::ReadMovement(glm::vec3 &mov) const noexcept {
	ReadPackN(mov, 42);
}

void Packet::PlayerUpdate::WriteActions(uint8_t actions) noexcept {
	Write(actions, 48);
}

void Packet::PlayerUpdate::ReadActions(uint8_t &actions) const noexcept {
	Read(actions, 48);
}

void Packet::PlayerUpdate::WriteSlot(uint8_t slot) noexcept {
	Write(slot, 49);
}

void Packet::PlayerUpdate::ReadSlot(uint8_t &slot) const noexcept {
	Read(slot, 49);
}

void Packet::SpawnEntity::WriteEntity(const Entity &e) noexcept {
	Write(e.ID(), 0);
	if (e.GetModel()) {
		Write(e.GetModel().GetModel().ID(), 4);
	} else {
		Write(uint32_t(0), 4);
	}
	Write(e.GetState(), 8);
	Write(e.Bounds(), 50);
	uint32_t flags = 0;
	if (e.WorldCollidable()) {
		flags |= 1;
	}
	Write(flags, 74);
	WriteString(e.Name(), 78, 32);
}

void Packet::SpawnEntity::ReadEntityID(uint32_t &id) const noexcept {
	Read(id, 0);
}

void Packet::SpawnEntity::ReadModelID(uint32_t &id) const noexcept {
	Read(id, 4);
}

void Packet::SpawnEntity::ReadEntity(Entity &e) const noexcept {
	EntityState state;
	AABB bounds;
	uint32_t flags = 0;
	string name;

	Read(state, 8);
	Read(bounds, 50);
	Read(flags, 74);
	ReadString(name, 78, 32);

	e.SetState(state);
	e.Bounds(bounds);
	e.WorldCollidable(flags & 1);
	e.Name(name);
}

void Packet::DespawnEntity::WriteEntityID(uint32_t id) noexcept {
	Write(id, 0);
}

void Packet::DespawnEntity::ReadEntityID(uint32_t &id) const noexcept {
	Read(id, 0);
}

void Packet::EntityUpdate::WriteEntityCount(uint32_t count) noexcept {
	Write(count, 0);
}

void Packet::EntityUpdate::ReadEntityCount(uint32_t &count) const noexcept {
	Read(count, 0);
}

void Packet::EntityUpdate::WriteChunkBase(const glm::ivec3 &base) noexcept {
	Write(base, 4);
}

void Packet::EntityUpdate::ReadChunkBase(glm::ivec3 &base) const noexcept {
	Read(base, 4);
}

void Packet::EntityUpdate::WriteEntity(const Entity &entity, const glm::ivec3 &base, uint32_t num) noexcept {
	uint32_t off = GetSize(num);

	Write(entity.ID(), off);
	Write(entity.GetState(), base, off + 4);
}

void Packet::EntityUpdate::ReadEntityID(uint32_t &id, uint32_t num) const noexcept {
	uint32_t off = GetSize(num);
	Read(id, off);
}

void Packet::EntityUpdate::ReadEntityState(EntityState &state, const glm::ivec3 &base, uint32_t num) const noexcept {
	uint32_t off = GetSize(num);
	Read(state, base, off + 4);
}

void Packet::PlayerCorrection::WritePacketSeq(std::uint16_t s) noexcept {
	Write(s, 0);
}

void Packet::PlayerCorrection::ReadPacketSeq(std::uint16_t &s) const noexcept {
	Read(s, 0);
}

void Packet::PlayerCorrection::WritePlayer(const Entity &player) noexcept {
	Write(player.GetState(), 2);
}

void Packet::PlayerCorrection::ReadPlayerState(EntityState &state) const noexcept {
	Read(state, 2);
}

void Packet::ChunkBegin::WriteTransmissionId(uint32_t id) noexcept {
	Write(id, 0);
}

void Packet::ChunkBegin::ReadTransmissionId(uint32_t &id) const noexcept {
	Read(id, 0);
}

void Packet::ChunkBegin::WriteFlags(uint32_t f) noexcept {
	Write(f, 4);
}

void Packet::ChunkBegin::ReadFlags(uint32_t &f) const noexcept {
	Read(f, 4);
}

void Packet::ChunkBegin::WriteChunkCoords(const glm::ivec3 &pos) noexcept {
	Write(pos, 8);
}

void Packet::ChunkBegin::ReadChunkCoords(glm::ivec3 &pos) const noexcept {
	Read(pos, 8);
}

void Packet::ChunkBegin::WriteDataSize(uint32_t s) noexcept {
	Write(s, 20);
}

void Packet::ChunkBegin::ReadDataSize(uint32_t &s) const noexcept {
	Read(s, 20);
}

void Packet::ChunkData::WriteTransmissionId(uint32_t id) noexcept {
	Write(id, 0);
}

void Packet::ChunkData::ReadTransmissionId(uint32_t &id) const noexcept {
	Read(id, 0);
}

void Packet::ChunkData::WriteDataOffset(uint32_t o) noexcept {
	Write(o, 4);
}

void Packet::ChunkData::ReadDataOffset(uint32_t &o) const noexcept {
	Read(o, 4);
}

void Packet::ChunkData::WriteDataSize(uint32_t s) noexcept {
	Write(s, 8);
}

void Packet::ChunkData::ReadDataSize(uint32_t &s) const noexcept {
	Read(s, 8);
}

void Packet::ChunkData::WriteData(const uint8_t *d, size_t l) noexcept {
	size_t len = min(length - 12, l);
	memcpy(&data[12], d, len);
}

void Packet::ChunkData::ReadData(uint8_t *d, size_t l) const noexcept {
	size_t len = min(length - 12, l);
	memcpy(d, &data[12], len);
}

void Packet::BlockUpdate::WriteChunkCoords(const glm::ivec3 &coords) noexcept {
	Write(coords, 0);
}

void Packet::BlockUpdate::ReadChunkCoords(glm::ivec3 &coords) const noexcept {
	Read(coords, 0);
}

void Packet::BlockUpdate::WriteBlockCount(uint32_t count) noexcept {
	Write(count, 12);
}

void Packet::BlockUpdate::ReadBlockCount(uint32_t &count) const noexcept {
	Read(count, 12);
}

void Packet::BlockUpdate::WriteIndex(uint16_t index, uint32_t num) noexcept {
	uint32_t off = GetSize(num);
	Write(index, off);
}

void Packet::BlockUpdate::ReadIndex(uint16_t &index, uint32_t num) const noexcept {
	uint32_t off = GetSize(num);
	Read(index, off);
}

void Packet::BlockUpdate::WriteBlock(const Block &block, uint32_t num) noexcept {
	uint32_t off = GetSize(num) + 2;
	Write(block, off);
}

void Packet::BlockUpdate::ReadBlock(Block &block, uint32_t num) const noexcept {
	uint32_t off = GetSize(num) + 2;
	Read(block, off);
}

void Packet::Message::WriteType(uint8_t type) noexcept {
	Write(type, 0);
}

void Packet::Message::ReadType(uint8_t &type) const noexcept {
	Read(type, 0);
}

void Packet::Message::WriteReferral(uint32_t ref) noexcept {
	Write(ref, 1);
}

void Packet::Message::ReadReferral(uint32_t &ref) const noexcept {
	Read(ref, 1);
}

void Packet::Message::WriteMessage(const string &msg) noexcept {
	WriteString(msg, 5, MAX_MESSAGE_LEN);
}

void Packet::Message::ReadMessage(string &msg) const noexcept {
	ReadString(msg, 5, MAX_MESSAGE_LEN);
}


void ConnectionHandler::Handle(const UDPpacket &udp_pack) {
	const Packet &pack = *reinterpret_cast<const Packet *>(udp_pack.data);
	switch (pack.Type()) {
		case Packet::Ping::TYPE:
			On(Packet::As<Packet::Ping>(udp_pack));
			break;
		case Packet::Login::TYPE:
			On(Packet::As<Packet::Login>(udp_pack));
			break;
		case Packet::Join::TYPE:
			On(Packet::As<Packet::Join>(udp_pack));
			break;
		case Packet::Part::TYPE:
			On(Packet::As<Packet::Part>(udp_pack));
			break;
		case Packet::PlayerUpdate::TYPE:
			On(Packet::As<Packet::PlayerUpdate>(udp_pack));
			break;
		case Packet::SpawnEntity::TYPE:
			On(Packet::As<Packet::SpawnEntity>(udp_pack));
			break;
		case Packet::DespawnEntity::TYPE:
			On(Packet::As<Packet::DespawnEntity>(udp_pack));
			break;
		case Packet::EntityUpdate::TYPE:
			On(Packet::As<Packet::EntityUpdate>(udp_pack));
			break;
		case Packet::PlayerCorrection::TYPE:
			On(Packet::As<Packet::PlayerCorrection>(udp_pack));
			break;
		case Packet::ChunkBegin::TYPE:
			On(Packet::As<Packet::ChunkBegin>(udp_pack));
			break;
		case Packet::ChunkData::TYPE:
			On(Packet::As<Packet::ChunkData>(udp_pack));
			break;
		case Packet::BlockUpdate::TYPE:
			On(Packet::As<Packet::BlockUpdate>(udp_pack));
			break;
		case Packet::Message::TYPE:
			On(Packet::As<Packet::Message>(udp_pack));
			break;
		default:
			// drop unknown or unhandled packets
			break;
	}
}

}
