#include "ChunkReceiver.hpp"
#include "ChunkTransmission.hpp"
#include "Client.hpp"

#include "../app/init.hpp"
#include "../net/Packet.hpp"
#include "../world/Chunk.hpp"
#include "../world/ChunkStore.hpp"

#include <iostream>
#include <zlib.h>
#include <glm/gtx/io.hpp>

using namespace std;


namespace blank {
namespace client {


ChunkReceiver::ChunkReceiver(ChunkStore &store)
: store(store)
, transmissions()
, timer(5000) {
	timer.Start();
}

ChunkReceiver::~ChunkReceiver() {

}

void ChunkReceiver::Update(int dt) {
	timer.Update(dt);
	for (ChunkTransmission &trans : transmissions) {
		if (trans.active && (timer.Elapsed() - trans.last_update) > timer.Interval()) {
			cout << "timeout for transmission of chunk " << trans.coords << endl;
			trans.Clear();
		}
	}
	if (transmissions.size() > 3) {
		for (auto iter = transmissions.begin(), end = transmissions.end(); iter != end; ++iter) {
			if (!iter->active) {
				transmissions.erase(iter);
				break;
			}
		}
	}
}

void ChunkReceiver::Handle(const Packet::ChunkBegin &pack) {
	uint32_t id;
	pack.ReadTransmissionId(id);
	ChunkTransmission &trans = GetTransmission(id);
	pack.ReadFlags(trans.flags);
	pack.ReadChunkCoords(trans.coords);
	pack.ReadDataSize(trans.data_size);
	trans.last_update = timer.Elapsed();
	trans.header_received = true;
	Commit(trans);
}

void ChunkReceiver::Handle(const Packet::ChunkData &pack) {
	uint32_t id, pos, size;
	pack.ReadTransmissionId(id);
	pack.ReadDataOffset(pos);
	if (pos >= sizeof(ChunkTransmission::buffer)) {
		cout << "received chunk data offset outside of buffer size" << endl;
		return;
	}
	pack.ReadDataSize(size);
	ChunkTransmission &trans = GetTransmission(id);
	size_t len = min(size_t(size), sizeof(ChunkTransmission::buffer) - pos);
	pack.ReadData(&trans.buffer[pos], len);
	// TODO: this method breaks when a packet arrives twice
	trans.data_received += len;
	trans.last_update = timer.Elapsed();
	Commit(trans);
}

ChunkTransmission &ChunkReceiver::GetTransmission(uint32_t id) {
	// search for ongoing
	for (ChunkTransmission &trans : transmissions) {
		if (trans.active && trans.id == id) {
			return trans;
		}
	}
	// search for unused
	for (ChunkTransmission &trans : transmissions) {
		if (!trans.active) {
			trans.active = true;
			trans.id = id;
			return trans;
		}
	}
	// allocate new
	transmissions.emplace_back();
	transmissions.back().active = true;
	transmissions.back().id = id;
	return transmissions.back();
}

void ChunkReceiver::Commit(ChunkTransmission &trans) {
	if (!trans.Complete()) return;

	Chunk *chunk = store.Allocate(trans.coords);
	if (!chunk) {
		// chunk no longer of interes, just drop the data
		// it should probably be cached to disk, but not now :P
		trans.Clear();
		return;
	}

	const Byte *src = &trans.buffer[0];
	uLong src_len = min(size_t(trans.data_size), sizeof(ChunkTransmission::buffer));
	Byte *dst = reinterpret_cast<Byte *>(chunk->BlockData());
	uLong dst_len = Chunk::BlockSize();

	if (trans.Compressed()) {
		if (uncompress(dst, &dst_len, src, src_len) != Z_OK) {
			// omg, now what?
			cout << "got corruped chunk data for " << trans.coords << endl;
		}
	} else {
		memcpy(dst, src, min(src_len, dst_len));
	}
	chunk->Invalidate();
	trans.Clear();
}

ChunkTransmission::ChunkTransmission()
: id(0)
, flags(0)
, coords()
, data_size(0)
, data_received(0)
, last_update(0)
, header_received(false)
, active(false)
, buffer() {

}

void ChunkTransmission::Clear() noexcept {
	data_size = 0;
	data_received = 0;
	last_update = 0;
	header_received = false;
	active = false;
}

bool ChunkTransmission::Complete() const noexcept {
	return header_received && data_received == data_size;
}

bool ChunkTransmission::Compressed() const noexcept {
	return flags & 1;
}


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

Client::Client(const Config::Network &conf)
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

}
}
