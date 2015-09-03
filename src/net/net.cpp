#include "Client.hpp"
#include "Connection.hpp"
#include "io.hpp"
#include "Packet.hpp"
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

Client::Client(const Config &conf, World &world)
: world(world)
, conn(client_resolve(conf.host.c_str(), conf.port))
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
	Packet &pack = *reinterpret_cast<Packet *>(client_pack.data);
	client_pack.len = pack.MakeLogin(name);
	conn.Send(client_pack, client_sock);
}


Connection::Connection(const IPaddress &addr)
: addr(addr)
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

	cout << "sending " << pack.GetType() << " to " << Address() << endl;

	udp_pack.address = addr;
	if (SDLNet_UDP_Send(sock, -1, &udp_pack) == 0) {
		throw NetError("SDLNet_UDP_Send");
	}

	FlagSend();
}

void Connection::Received(const UDPpacket &udp_pack) {
	Packet &pack = *reinterpret_cast<Packet *>(udp_pack.data);

	cout << "received " << pack.GetType() << " from " << Address() << endl;

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

	FlagRecv();
}

void Connection::SendPing(UDPpacket &udp_pack, UDPsocket sock) {
	Packet &pack = *reinterpret_cast<Packet *>(udp_pack.data);
	udp_pack.len = pack.MakePing();
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


const char *Packet::Type2String(Type t) noexcept {
	switch (t) {
		case PING:
			return "PING";
		case LOGIN:
			return "LOGIN";
		case JOIN:
			return "JOIN";
		case PART:
			return "PART";
		default:
			return "UNKNOWN";
	}
}

void Packet::Tag() noexcept {
	header.tag = TAG;
}

size_t Packet::MakePing() noexcept {
	Tag();
	header.type = PING;
	return sizeof(Header);
}

size_t Packet::MakeLogin(const string &name) noexcept {
	constexpr size_t maxname = 32;

	Tag();
	header.type = LOGIN;
	if (name.size() < maxname) {
		memset(payload, '\0', maxname);
		memcpy(payload, name.c_str(), name.size());
	} else {
		memcpy(payload, name.c_str(), maxname);
	}
	return sizeof(Header) + maxname;
}

size_t Packet::MakeJoin(const Entity &player, const string &world_name) noexcept {
	constexpr size_t maxname = 32;

	Tag();
	header.type = JOIN;

	uint8_t *cursor = &payload[0];

	// TODO: generate entity IDs
	*reinterpret_cast<uint32_t *>(cursor) = 1;
	cursor += 4;

	*reinterpret_cast<glm::ivec3 *>(cursor) = player.ChunkCoords();
	cursor += 12;

	*reinterpret_cast<glm::vec3 *>(cursor) = player.Position();
	cursor += 12;
	*reinterpret_cast<glm::vec3 *>(cursor) = player.Velocity();
	cursor += 12;

	*reinterpret_cast<glm::quat *>(cursor) = player.Orientation();
	cursor += 16;
	*reinterpret_cast<glm::vec3 *>(cursor) = player.AngularVelocity();
	cursor += 12;

	if (world_name.size() < maxname) {
		memset(cursor, '\0', maxname);
		memcpy(cursor, world_name.c_str(), world_name.size());
	} else {
		memcpy(cursor, world_name.c_str(), maxname);
	}
	cursor += maxname;

	return sizeof(Header) + (cursor - &payload[0]);
}

size_t Packet::MakePart() noexcept {
	Tag();
	header.type = PART;
	return sizeof(Header);
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
		case Packet::LOGIN:
			HandleLogin(client, udp_pack);
			break;
		case Packet::PART:
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
	const Packet &pack = *reinterpret_cast<const Packet *>(udp_pack.data);
	size_t maxlen = min(udp_pack.len - int(sizeof(Packet::Header)), 32);
	string name;
	name.reserve(maxlen);
	for (size_t i = 0; i < maxlen && pack.payload[i] != '\0'; ++i) {
		name.push_back(pack.payload[i]);
	}

	Entity *player = world.AddPlayer(name);
	Packet &response = *reinterpret_cast<Packet *>(serv_pack.data);

	if (player) {
		// success!
		cout << "accepted login from player \"" << name << '"' << endl;
		response.MakeJoin(*player, world.Name());
		client.Send(serv_pack, serv_sock);
	} else {
		// aw no :(
		cout << "rejected login from player \"" << name << '"' << endl;
		response.MakePart();
		client.Send(serv_pack, serv_sock);
		client.Close();
	}
}

void Server::HandlePart(Connection &client, const UDPpacket &udp_pack) {
	client.Close();
}

}
