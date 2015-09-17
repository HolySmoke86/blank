#ifndef BLANK_NET_CHUNKRECEIVER_HPP_
#define BLANK_NET_CHUNKRECEIVER_HPP_

#include "Packet.hpp"
#include "../app/IntervalTimer.hpp"

#include <cstdint>
#include <list>


namespace blank {

class ChunkStore;
class ChunkTransmission;

class ChunkReceiver {

public:
	explicit ChunkReceiver(ChunkStore &);
	~ChunkReceiver();

	void Update(int dt);

	void Handle(const Packet::ChunkBegin &);
	void Handle(const Packet::ChunkData &);

private:
	ChunkTransmission &GetTransmission(std::uint32_t id);
	void Commit(ChunkTransmission &);

private:
	ChunkStore &store;
	std::list<ChunkTransmission> transmissions;
	IntervalTimer timer;

};

}

#endif
