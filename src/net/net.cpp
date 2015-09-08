#include "Client.hpp"
#include "ClientConnection.hpp"
#include "Connection.hpp"
#include "ConnectionHandler.hpp"
#include "io.hpp"
#include "Packet.hpp"
#include "Server.hpp"

#include "../app/init.hpp"
#include "../world/World.hpp"

#include <cstring>
#include <iostream>
#include <glm/gtx/io.hpp>

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

namespace {

UDPsocket client_bind(Uint16 port) {
	UDPsocket sock = SDLNet_UDP_Open(port);
	if (!sock) {
		throw NetError("SDLNet_UDP_Open");
	}
	return sock;
}

IPaddress client_resolve(const char *host, Uint16 port) {
	IPaddress addr;
	if (SDLNet_ResolveHost(&addr, host, port) != 0) {
		throw NetError("SDLNet_ResolveHost");
	}
	return addr;
}

}

Client::Client(const Config &conf)
: conn(client_resolve(conf.host.c_str(), conf.port))
, client_sock(client_bind(0))
, client_pack{ -1, nullptr, 0 } {
	client_pack.data = new Uint8[sizeof(Packet)];
	client_pack.maxlen = sizeof(Packet);
	// establish connection
	SendPing();
}

Client::~Client() {
	delete[] client_pack.data;
	SDLNet_UDP_Close(client_sock);
}


void Client::Handle() {
	int result = SDLNet_UDP_Recv(client_sock, &client_pack);
	while (result > 0) {
		HandlePacket(client_pack);
		result = SDLNet_UDP_Recv(client_sock, &client_pack);
	}
	if (result == -1) {
		// a boo boo happened
		throw NetError("SDLNet_UDP_Recv");
	}
}

void Client::HandlePacket(const UDPpacket &udp_pack) {
	if (!conn.Matches(udp_pack.address)) {
		// packet came from somewhere else, drop
		return;
	}
	const Packet &pack = *reinterpret_cast<const Packet *>(udp_pack.data);
	if (pack.header.tag != Packet::TAG) {
		// mistagged packet, drop
		return;
	}

	conn.Received(udp_pack);
}

void Client::Update(int dt) {
	conn.Update(dt);
	if (conn.ShouldPing()) {
		SendPing();
	}
}

uint16_t Client::SendPing() {
	return conn.SendPing(client_pack, client_sock);
}

uint16_t Client::SendLogin(const string &name) {
	auto pack = Packet::Make<Packet::Login>(client_pack);
	pack.WritePlayerName(name);
	return conn.Send(client_pack, client_sock);
}

uint16_t Client::SendPlayerUpdate(const Entity &player) {
	auto pack = Packet::Make<Packet::PlayerUpdate>(client_pack);
	pack.WritePlayer(player);
	return conn.Send(client_pack, client_sock);
}

uint16_t Client::SendPart() {
	Packet::Make<Packet::Part>(client_pack);
	return conn.Send(client_pack, client_sock);
}


ClientConnection::ClientConnection(Server &server, const IPaddress &addr)
: server(server)
, conn(addr)
, player(nullptr)
, spawns()
, confirm_wait(0) {
	conn.SetHandler(this);
}

ClientConnection::~ClientConnection() {
	DetachPlayer();
}

void ClientConnection::Update(int dt) {
	conn.Update(dt);
	if (Disconnected()) {
		return;
	}
	if (HasPlayer()) {
		// sync entities
		auto global_iter = server.GetWorld().Entities().begin();
		auto global_end = server.GetWorld().Entities().end();
		auto local_iter = spawns.begin();
		auto local_end = spawns.end();

		while (global_iter != global_end && local_iter != local_end) {
			if (global_iter->ID() == local_iter->entity->ID()) {
				// they're the same
				if (CanDespawn(*global_iter)) {
					SendDespawn(*local_iter);
				} else {
					// update
					SendUpdate(*local_iter);
				}
				++global_iter;
				++local_iter;
			} else if (global_iter->ID() < local_iter->entity->ID()) {
				// global entity was inserted
				if (CanSpawn(*global_iter)) {
					auto spawned = spawns.emplace(local_iter, *global_iter);
					SendSpawn(*spawned);
				}
				++global_iter;
			} else {
				// global entity was removed
				SendDespawn(*local_iter);
				++local_iter;
			}
		}

		// leftover spawns
		while (global_iter != global_end) {
			if (CanSpawn(*global_iter)) {
				spawns.emplace_back(*global_iter);
				SendSpawn(spawns.back());
			}
			++global_iter;
		}

		// leftover despawns
		while (local_iter != local_end) {
			SendDespawn(*local_iter);
			++local_iter;
		}
	}
	if (conn.ShouldPing()) {
		conn.SendPing(server.GetPacket(), server.GetSocket());
	}
}

ClientConnection::SpawnStatus::SpawnStatus(Entity &e)
: entity(&e)
, spawn_pack(-1)
, despawn_pack(-1) {
	entity->Ref();
}

ClientConnection::SpawnStatus::~SpawnStatus() {
	entity->UnRef();
}

bool ClientConnection::CanSpawn(const Entity &e) const noexcept {
	return
		&e != player &&
		!e.Dead() &&
		manhattan_radius(e.ChunkCoords() - Player().ChunkCoords()) < 7;
}

bool ClientConnection::CanDespawn(const Entity &e) const noexcept {
	return
		e.Dead() ||
		manhattan_radius(e.ChunkCoords() - Player().ChunkCoords()) > 7;
}

void ClientConnection::SendSpawn(SpawnStatus &status) {
	// don't double spawn
	if (status.spawn_pack != -1) return;

	auto pack = Packet::Make<Packet::SpawnEntity>(server.GetPacket());
	pack.WriteEntity(*status.entity);
	status.spawn_pack = conn.Send(server.GetPacket(), server.GetSocket());
	++confirm_wait;
}

void ClientConnection::SendDespawn(SpawnStatus &status) {
	// don't double despawn
	if (status.despawn_pack != -1) return;

	auto pack = Packet::Make<Packet::DespawnEntity>(server.GetPacket());
	pack.WriteEntityID(status.entity->ID());
	status.despawn_pack = conn.Send(server.GetPacket(), server.GetSocket());
	++confirm_wait;
}

void ClientConnection::SendUpdate(SpawnStatus &status) {
	// don't send updates while spawn not ack'd or despawn sent
	if (status.spawn_pack != -1 || status.despawn_pack != -1) return;

	// TODO: pack entity updates
	auto pack = Packet::Make<Packet::EntityUpdate>(server.GetPacket());
	pack.WriteEntityCount(1);
	pack.WriteEntity(*status.entity, 0);
	server.GetPacket().len = Packet::EntityUpdate::GetSize(1);
	conn.Send(server.GetPacket(), server.GetSocket());
}

void ClientConnection::AttachPlayer(Entity &new_player) {
	DetachPlayer();
	player = &new_player;
	player->Ref();
	cout << "player \"" << player->Name() << "\" joined" << endl;
}

void ClientConnection::DetachPlayer() {
	if (!player) return;
	player->Kill();
	player->UnRef();
	cout << "player \"" << player->Name() << "\" left" << endl;
	player = nullptr;
}

void ClientConnection::OnPacketReceived(uint16_t seq) {
	if (!confirm_wait) return;
	for (auto iter = spawns.begin(), end = spawns.end(); iter != end; ++iter) {
		if (seq == iter->spawn_pack) {
			iter->spawn_pack = -1;
			--confirm_wait;
			return;
		}
		if (seq == iter->despawn_pack) {
			spawns.erase(iter);
			--confirm_wait;
			return;
		}
	}
}

void ClientConnection::OnPacketLost(uint16_t seq) {
	if (!confirm_wait) return;
	for (SpawnStatus &status : spawns) {
		if (seq == status.spawn_pack) {
			status.spawn_pack = -1;
			--confirm_wait;
			SendSpawn(status);
			return;
		}
		if (seq == status.despawn_pack) {
			status.despawn_pack = -1;
			--confirm_wait;
			SendDespawn(status);
			return;
		}
	}
}

void ClientConnection::On(const Packet::Login &pack) {
	string name;
	pack.ReadPlayerName(name);

	Entity *new_player = server.GetWorld().AddPlayer(name);

	if (new_player) {
		// success!
		AttachPlayer(*new_player);
		cout << "accepted login from player \"" << name << '"' << endl;
		auto response = Packet::Make<Packet::Join>(server.GetPacket());
		response.WritePlayer(*new_player);
		response.WriteWorldName(server.GetWorld().Name());
		conn.Send(server.GetPacket(), server.GetSocket());
	} else {
		// aw no :(
		cout << "rejected login from player \"" << name << '"' << endl;
		Packet::Make<Packet::Part>(server.GetPacket());
		conn.Send(server.GetPacket(), server.GetSocket());
		conn.Close();
	}
}

void ClientConnection::On(const Packet::Part &) {
	conn.Close();
}

void ClientConnection::On(const Packet::PlayerUpdate &pack) {
	if (!HasPlayer()) return;
	pack.ReadPlayer(Player());
}


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
					Handler().OnPacketLost(ctrl_in.ack - 32 + i);
				}
			}
		}
		// check for newly ack'd packets
		for (uint16_t s = ctrl_new.AckBegin(); s != ctrl_new.AckEnd(); ++s) {
			if (ctrl_new.Acks(s) && !ctrl_in.Acks(s)) {
				Handler().OnPacketReceived(s);
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


void Packet::Login::WritePlayerName(const string &name) noexcept {
	WriteString(name, 0, 32);
}

void Packet::Login::ReadPlayerName(string &name) const noexcept {
	ReadString(name, 0, 32);
}

void Packet::Join::WritePlayer(const Entity &player) noexcept {
	Write(player.ID(), 0);
	Write(player.ChunkCoords(), 4);
	Write(player.Position(), 16);
	Write(player.Velocity(), 28);
	Write(player.Orientation(), 40);
	Write(player.AngularVelocity(), 56);
}

void Packet::Join::ReadPlayerID(uint32_t &id) const noexcept {
	Read(id, 0);
}

void Packet::Join::ReadPlayer(Entity &player) const noexcept {
	glm::ivec3 chunk_coords(0);
	glm::vec3 pos;
	glm::vec3 vel;
	glm::quat rot;
	glm::vec3 ang;

	Read(chunk_coords, 4);
	Read(pos, 16);
	Read(vel, 28);
	Read(rot, 40);
	Read(ang, 56);

	player.Position(chunk_coords, pos);
	player.Velocity(vel);
	player.Orientation(rot);
	player.AngularVelocity(ang);
}

void Packet::Join::WriteWorldName(const string &name) noexcept {
	WriteString(name, 68, 32);
}

void Packet::Join::ReadWorldName(string &name) const noexcept {
	ReadString(name, 68, 32);
}

void Packet::PlayerUpdate::WritePlayer(const Entity &player) noexcept {
	Write(player.ChunkCoords(), 0);
	Write(player.Position(), 12);
	Write(player.Velocity(), 24);
	Write(player.Orientation(), 36);
	Write(player.AngularVelocity(), 52);
}

void Packet::PlayerUpdate::ReadPlayer(Entity &player) const noexcept {
	glm::ivec3 chunk_coords(0);
	glm::vec3 pos;
	glm::vec3 vel;
	glm::quat rot;
	glm::vec3 ang;

	Read(chunk_coords, 0);
	Read(pos, 12);
	Read(vel, 24);
	Read(rot, 36);
	Read(ang, 52);

	player.Position(chunk_coords, pos);
	player.Velocity(vel);
	player.Orientation(rot);
	player.AngularVelocity(ang);
}

void Packet::SpawnEntity::WriteEntity(const Entity &e) noexcept {
	Write(e.ID(), 0);
	Write(e.ChunkCoords(), 4);
	Write(e.Position(), 16);
	Write(e.Velocity(), 28);
	Write(e.Orientation(), 40);
	Write(e.AngularVelocity(), 56);
	Write(e.Bounds(), 68);
	uint32_t flags = 0;
	if (e.WorldCollidable()) {
		flags |= 1;
	}
	Write(flags, 92);
	WriteString(e.Name(), 96, 32);
}

void Packet::SpawnEntity::ReadEntityID(uint32_t &id) const noexcept {
	Read(id, 0);
}

void Packet::SpawnEntity::ReadEntity(Entity &e) const noexcept {
	glm::ivec3 chunk_coords(0);
	glm::vec3 pos;
	glm::vec3 vel;
	glm::quat rot;
	glm::vec3 ang;
	AABB bounds;
	uint32_t flags = 0;
	string name;

	Read(chunk_coords, 4);
	Read(pos, 16);
	Read(vel, 28);
	Read(rot, 40);
	Read(ang, 56);
	Read(bounds, 68);
	Read(flags, 92);
	ReadString(name, 96, 32);

	e.Position(chunk_coords, pos);
	e.Velocity(vel);
	e.Orientation(rot);
	e.AngularVelocity(ang);
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

void Packet::EntityUpdate::WriteEntity(const Entity &entity, uint32_t num) noexcept {
	uint32_t off = 4 + (num * 64);

	Write(entity.ID(), off);
	Write(entity.ChunkCoords(), off + 4);
	Write(entity.Position(), off + 16);
	Write(entity.Velocity(), off + 28);
	Write(entity.Orientation(), off + 40);
	Write(entity.AngularVelocity(), off + 56);
}

void Packet::EntityUpdate::ReadEntityID(uint32_t &id, uint32_t num) const noexcept {
	Read(id, 4 + (num * 64));
}

void Packet::EntityUpdate::ReadEntity(Entity &entity, uint32_t num) const noexcept {
	uint32_t off = 4 + (num * 64);

	glm::ivec3 chunk_coords(0);
	glm::vec3 pos;
	glm::vec3 vel;
	glm::quat rot;
	glm::vec3 ang;

	Read(chunk_coords, off + 4);
	Read(pos, off + 16);
	Read(vel, off + 28);
	Read(rot, off + 40);
	Read(ang, off + 56);

	entity.Position(chunk_coords, pos);
	entity.Velocity(vel);
	entity.Orientation(rot);
	entity.AngularVelocity(ang);
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
		default:
			// drop unknown or unhandled packets
			break;
	}
}


Server::Server(const Config &conf, World &world)
: serv_sock(nullptr)
, serv_pack{ -1, nullptr, 0 }
, clients()
, world(world) {
	serv_sock = SDLNet_UDP_Open(conf.port);
	if (!serv_sock) {
		throw NetError("SDLNet_UDP_Open");
	}

	serv_pack.data = new Uint8[sizeof(Packet)];
	serv_pack.maxlen = sizeof(Packet);
}

Server::~Server() {
	delete[] serv_pack.data;
	SDLNet_UDP_Close(serv_sock);
}


void Server::Handle() {
	int result = SDLNet_UDP_Recv(serv_sock, &serv_pack);
	while (result > 0) {
		HandlePacket(serv_pack);
		result = SDLNet_UDP_Recv(serv_sock, &serv_pack);
	}
	if (result == -1) {
		// a boo boo happened
		throw NetError("SDLNet_UDP_Recv");
	}
}

void Server::HandlePacket(const UDPpacket &udp_pack) {
	if (udp_pack.len < int(sizeof(Packet::Header))) {
		// packet too small, drop
		return;
	}
	const Packet &pack = *reinterpret_cast<const Packet *>(udp_pack.data);
	if (pack.header.tag != Packet::TAG) {
		// mistagged packet, drop
		return;
	}

	ClientConnection &client = GetClient(udp_pack.address);
	client.GetConnection().Received(udp_pack);
}

ClientConnection &Server::GetClient(const IPaddress &addr) {
	for (ClientConnection &client : clients) {
		if (client.Matches(addr)) {
			return client;
		}
	}
	clients.emplace_back(*this, addr);
	return clients.back();
}

void Server::Update(int dt) {
	for (list<ClientConnection>::iterator client(clients.begin()), end(clients.end()); client != end;) {
		client->Update(dt);
		if (client->Disconnected()) {
			client = clients.erase(client);
		} else {
			++client;
		}
	}
}

}
