#ifndef BLANK_CLIENT_CHUNKRECEIVER_HPP_
#define BLANK_CLIENT_CHUNKRECEIVER_HPP_

#include "../app/IntervalTimer.hpp"
#include "../net/Packet.hpp"

#include <cstdint>
#include <list>


namespace blank {

class ChunkStore;

namespace client {

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
}

#endif
