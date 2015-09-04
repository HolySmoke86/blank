#include "Client.hpp"
#include "Connection.hpp"
#include "io.hpp"
#include "Packet.hpp"
#include "PacketHandler.hpp"
#include "Server.hpp"

#include "../app/init.hpp"
#include "../world/World.hpp"

#include <cstring>
#include <iostream>

using namespace std;


namespace blank {

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
	if (conn.TimedOut()) {
		cout << "connection timed out :(" << endl;
	} else if (conn.ShouldPing()) {
		SendPing();
	}
}

void Client::SendPing() {
	conn.SendPing(client_pack, client_sock);
}

void Client::SendLogin(const string &name) {
	auto pack = Packet::Make<Packet::Login>(client_pack);
	pack.WritePlayerName(name);
	conn.Send(client_pack, client_sock);
}


Connection::Connection(const IPaddress &addr)
: handler(nullptr)
, addr(addr)
, send_timer(3000)
, recv_timer(10000)
, ctrl{ 0, 0xFFFF, 0xFFFF }
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
	return send_timer.HitOnce();
}

bool Connection::TimedOut() const noexcept {
	return recv_timer.HitOnce();
}

void Connection::Update(int dt) {
	send_timer.Update(dt);
	recv_timer.Update(dt);
}


void Connection::Send(UDPpacket &udp_pack, UDPsocket sock) {
	Packet &pack = *reinterpret_cast<Packet *>(udp_pack.data);
	pack.header.ctrl = ctrl;
	++ctrl.seq;

	cout << "sending " << pack.TypeString() << " to " << Address() << endl;

	udp_pack.address = addr;
	if (SDLNet_UDP_Send(sock, -1, &udp_pack) == 0) {
		throw NetError("SDLNet_UDP_Send");
	}

	FlagSend();
}

void Connection::Received(const UDPpacket &udp_pack) {
	Packet &pack = *reinterpret_cast<Packet *>(udp_pack.data);

	cout << "received " << pack.TypeString() << " from " << Address() << endl;

	int diff = std::int16_t(pack.header.ctrl.seq) - std::int16_t(ctrl.ack);

	if (diff > 0) {
		// incoming more recent than last acked

		// TODO: packets considered lost are detected here
		//       this should have ones for all of them:
		//       ~hist & ((1 << dist) - 1) if dist is < 32

		if (diff >= 32) {
			// missed more than the last 32 oO
			ctrl.hist = 0;
		} else {
			ctrl.hist >>= diff;
			ctrl.hist |= 1 << (32 - diff);
		}
	} else if (diff < 0) {
		// incoming older than acked
		if (diff > -32) {
			// too late :/
		} else {
			ctrl.hist |= 1 << (32 + diff);
		}
	} else {
		// incoming the same as last acked oO
	}

	ctrl.ack = pack.header.ctrl.seq;

	if (HasHandler()) {
		Handler().Handle(udp_pack);
	}

	FlagRecv();
}

void Connection::SendPing(UDPpacket &udp_pack, UDPsocket sock) {
	Packet::Make<Packet::Ping>(udp_pack);
	Send(udp_pack, sock);
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
	// TODO: generate entity IDs
	Write(uint32_t(1), 0);
	Write(player.ChunkCoords(), 4);
	Write(player.Position(), 16);
	Write(player.Velocity(), 28);
	Write(player.Orientation(), 40);
	Write(player.AngularVelocity(), 56);
}

void Packet::Join::ReadPlayer(Entity &player) const noexcept {
	uint32_t id = 0;
	glm::ivec3 chunk_coords(0);
	glm::vec3 pos;
	glm::vec3 vel;
	glm::quat rot;
	glm::vec3 ang;

	Read(id, 0);
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


void PacketHandler::Handle(const UDPpacket &udp_pack) {
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

	Connection &client = GetClient(udp_pack.address);
	client.Received(udp_pack);

	switch (pack.header.type) {
		case Packet::Login::TYPE:
			HandleLogin(client, udp_pack);
			break;
		case Packet::Part::TYPE:
			HandlePart(client, udp_pack);
			break;
		default:
			// just drop packets of unknown or unhandled type
			break;
	}
}

Connection &Server::GetClient(const IPaddress &addr) {
	for (Connection &client : clients) {
		if (client.Matches(addr)) {
			return client;
		}
	}
	clients.emplace_back(addr);
	OnConnect(clients.back());
	return clients.back();
}

void Server::OnConnect(Connection &client) {
	cout << "new connection from " << client.Address() << endl;
	// tell it we're alive
	client.SendPing(serv_pack, serv_sock);
}

void Server::Update(int dt) {
	for (list<Connection>::iterator client(clients.begin()), end(clients.end()); client != end;) {
		client->Update(dt);
		if (client->Closed()) {
			OnDisconnect(*client);
			client = clients.erase(client);
		} else {
			if (client->ShouldPing()) {
				client->SendPing(serv_pack, serv_sock);
			}
			++client;
		}
	}
}

void Server::OnDisconnect(Connection &client) {
	cout << "connection timeout from " << client.Address() << endl;
}


void Server::HandleLogin(Connection &client, const UDPpacket &udp_pack) {
	auto pack = Packet::As<Packet::Login>(udp_pack);

	string name;
	pack.ReadPlayerName(name);

	Entity *player = world.AddPlayer(name);

	if (player) {
		// success!
		cout << "accepted login from player \"" << name << '"' << endl;
		auto response = Packet::Make<Packet::Join>(serv_pack);
		response.WritePlayer(*player);
		response.WriteWorldName(world.Name());
		client.Send(serv_pack, serv_sock);
	} else {
		// aw no :(
		cout << "rejected login from player \"" << name << '"' << endl;
		Packet::Make<Packet::Part>(serv_pack);
		client.Send(serv_pack, serv_sock);
		client.Close();
	}
}

void Server::HandlePart(Connection &client, const UDPpacket &udp_pack) {
	client.Close();
}

}
