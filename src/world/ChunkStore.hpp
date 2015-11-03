#ifndef BLANK_WORLD_CHUNKSTORE_HPP_
#define BLANK_WORLD_CHUNKSTORE_HPP_

#include "Chunk.hpp"

#include <list>


namespace blank {

class ChunkIndex;

class ChunkStore {

public:
	ChunkStore(const BlockTypeRegistry &);
	~ChunkStore();

	ChunkStore(const ChunkStore &) = delete;
	ChunkStore &operator =(const ChunkStore &) = delete;

public:
	ChunkIndex &MakeIndex(const ExactLocation::Coarse &base, int extent);
	void UnregisterIndex(ChunkIndex &);

	ChunkIndex *ClosestIndex(const ExactLocation::Coarse &pos);

	/// returns nullptr if given position is not loaded
	Chunk *Get(const ExactLocation::Coarse &);
	/// returns nullptr if given position is not indexed
	Chunk *Allocate(const ExactLocation::Coarse &);

	std::list<Chunk>::iterator begin() noexcept { return loaded.begin(); }
	std::list<Chunk>::iterator end() noexcept { return loaded.end(); }

	std::size_t NumLoaded() const noexcept { return loaded.size(); }

	/// returns true if one of the indices is incomplete
	bool HasMissing() const noexcept;
	/// get the total number of missing chunks
	/// this is an estimate and represents the upper bound since
	/// chunks may be counted more than once if indices overlap
	int EstimateMissing() const noexcept;

	/// get coordinates of a missing chunk
	/// this will return garbage if none are actually missing
	ExactLocation::Coarse NextMissing() noexcept;

	void Clean();

private:
	const BlockTypeRegistry &types;

	std::list<Chunk> loaded;
	std::list<Chunk> free;

	std::list<ChunkIndex> indices;

};

}

#endif
