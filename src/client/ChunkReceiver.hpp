#ifndef BLANK_CLIENT_CHUNKRECEIVER_HPP_
#define BLANK_CLIENT_CHUNKRECEIVER_HPP_

#include "../app/IntervalTimer.hpp"
#include "../net/Packet.hpp"

#include <cstdint>
#include <list>


namespace blank {

class ChunkStore;
class WorldSave;

namespace client {

class ChunkTransmission;

class ChunkReceiver {

public:
	ChunkReceiver(ChunkStore &, const WorldSave &);
	~ChunkReceiver();

	void Update(int dt);

	int ToLoad() const noexcept;

	void LoadOne();
	void LoadN(std::size_t n);

	void StoreN(std::size_t n);

	void Handle(const Packet::ChunkBegin &);
	void Handle(const Packet::ChunkData &);

private:
	ChunkTransmission &GetTransmission(std::uint32_t id);
	void Commit(ChunkTransmission &);

private:
	ChunkStore &store;
	const WorldSave &save;
	std::list<ChunkTransmission> transmissions;
	CoarseTimer timer;

};

}
}

#endif
