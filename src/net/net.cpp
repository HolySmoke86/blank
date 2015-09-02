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

	conn.FlagRecv();
	cout << "I got something!" << endl;
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
	client_pack.len = pack.Login(name);
	conn.Send(client_pack, client_sock);
}


Connection::Connection(const IPaddress &addr)
: addr(addr)
, send_timer(5000)
, recv_timer(10000) {
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


void Connection::Send(UDPpacket &pack, UDPsocket sock) {
	pack.address = addr;
	if (SDLNet_UDP_Send(sock, -1, &pack) == 0) {
		throw NetError("SDLNet_UDP_Send");
	}
	FlagSend();
}

void Connection::SendPing(UDPpacket &udp_pack, UDPsocket sock) {
	Packet &pack = *reinterpret_cast<Packet *>(udp_pack.data);
	udp_pack.len = pack.Ping();
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


void Packet::Tag() noexcept {
	header.tag = TAG;
}

size_t Packet::Ping() noexcept {
	Tag();
	header.type = PING;
	return sizeof(Header);
}

size_t Packet::Login(const string &name) noexcept {
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
	client.FlagRecv();

	switch (pack.header.type) {
		case Packet::PING:
			// already done all that's supposed to do
			break;
		case Packet::LOGIN:
			HandleLogin(client, udp_pack);
			break;
		default:
			// just drop packets of unknown type
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
		if (client->TimedOut()) {
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
	cout << "got login request from player \"" << name << '"' << endl;

	Entity *player = world.AddPlayer(name);
	if (player) {
		// success!
		cout << "\taccepted" << endl;
	} else {
		// aw no :(
		cout << "\trejected" << endl;
	}
}

}
