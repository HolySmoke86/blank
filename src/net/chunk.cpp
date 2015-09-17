#include "ChunkReceiver.hpp"
#include "ChunkTransmission.hpp"
#include "ChunkTransmitter.hpp"

#include "ClientConnection.hpp"
#include "Packet.hpp"
#include "../world/Chunk.hpp"
#include "../world/ChunkStore.hpp"

#include <iostream>
#include <zlib.h>
#include <glm/gtx/io.hpp>

using namespace std;


namespace blank {

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


ChunkTransmitter::ChunkTransmitter(ClientConnection &conn)
: conn(conn)
, current(nullptr)
, buffer_size(Chunk::BlockSize() + 10)
, buffer(new uint8_t[buffer_size])
, buffer_len(0)
, packet_len(Packet::ChunkData::MAX_DATA_LEN)
, cursor(0)
, num_packets(0)
, begin_packet(-1)
, data_packets()
, confirm_wait(0)
, trans_id(0)
, compressed(false) {

}

ChunkTransmitter::~ChunkTransmitter() {
	Abort();
}

bool ChunkTransmitter::Idle() const noexcept {
	return !Transmitting() && !Waiting();
}

bool ChunkTransmitter::Transmitting() const noexcept {
	return cursor < num_packets;
}

void ChunkTransmitter::Transmit() {
	if (cursor < num_packets) {
		SendData(cursor);
		++cursor;
	}
}

bool ChunkTransmitter::Waiting() const noexcept {
	return confirm_wait > 0;
}

void ChunkTransmitter::Ack(uint16_t seq) {
	if (!Waiting()) {
		return;
	}
	if (seq == begin_packet) {
		begin_packet = -1;
		--confirm_wait;
		if (Idle()) {
			Release();
		}
		return;
	}
	for (int i = 0, end = data_packets.size(); i < end; ++i) {
		if (seq == data_packets[i]) {
			data_packets[i] = -1;
			--confirm_wait;
			if (Idle()) {
				Release();
			}
			return;
		}
	}
}

void ChunkTransmitter::Nack(uint16_t seq) {
	if (!Waiting()) {
		return;
	}
	if (seq == begin_packet) {
		SendBegin();
		return;
	}
	for (size_t i = 0, end = data_packets.size(); i < end; ++i) {
		if (seq == data_packets[i]) {
			SendData(i);
			return;
		}
	}
}

void ChunkTransmitter::Abort() {
	if (!current) return;

	Release();

	begin_packet = -1;
	data_packets.clear();
	confirm_wait = 0;
}

void ChunkTransmitter::Send(Chunk &chunk) {
	// abort current chunk, if any
	Abort();

	current = &chunk;
	current->Ref();

	// load new chunk data
	compressed = true;
	buffer_len = buffer_size;
	if (compress(buffer.get(), &buffer_len, reinterpret_cast<const Bytef *>(chunk.BlockData()), Chunk::BlockSize()) != Z_OK) {
		// compression failed, send it uncompressed
		buffer_len = Chunk::BlockSize();
		memcpy(buffer.get(), chunk.BlockData(), buffer_len);
		compressed = false;
	}
	cursor = 0;
	num_packets = (buffer_len / packet_len) + (buffer_len % packet_len != 0);
	data_packets.resize(num_packets, -1);

	++trans_id;
	SendBegin();
}

void ChunkTransmitter::SendBegin() {
	uint32_t flags = compressed;
	auto pack = conn.Prepare<Packet::ChunkBegin>();
	pack.WriteTransmissionId(trans_id);
	pack.WriteFlags(flags);
	pack.WriteChunkCoords(current->Position());
	pack.WriteDataSize(buffer_len);
	if (begin_packet == -1) {
		++confirm_wait;
	}
	begin_packet = conn.Send();
}

void ChunkTransmitter::SendData(size_t i) {
	int pos = i * packet_len;
	int len = min(packet_len, buffer_len - pos);
	const uint8_t *data = &buffer[pos];

	auto pack = conn.Prepare<Packet::ChunkData>();
	pack.WriteTransmissionId(trans_id);
	pack.WriteDataOffset(pos);
	pack.WriteDataSize(len);
	pack.WriteData(data, len);

	if (data_packets[i] == -1) {
		++confirm_wait;
	}
	data_packets[i] = conn.Send();
}

void ChunkTransmitter::Release() {
	if (current) {
		current->UnRef();
		current = nullptr;
	}
}

}
