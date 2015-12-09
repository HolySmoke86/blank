#ifndef BLANK_WORLD_CHUNKINDEX_HPP_
#define BLANK_WORLD_CHUNKINDEX_HPP_

#include "BlockLookup.hpp"
#include "Chunk.hpp"
#include "../rand/GaloisLFSR.hpp"

#include <vector>


namespace blank {

class ChunkStore;

class ChunkIndex {

public:
	ChunkIndex(ChunkStore &, const ExactLocation::Coarse &base, int extent);
	~ChunkIndex();

	ChunkIndex(const ChunkIndex &) = delete;
	ChunkIndex &operator =(const ChunkIndex &) = delete;

public:
	bool InRange(const ExactLocation::Coarse &) const noexcept;
	bool IsBorder(const ExactLocation::Coarse &) const noexcept;
	int Distance(const ExactLocation::Coarse &) const noexcept;

	bool HasAllSurrounding(const ExactLocation::Coarse &) const noexcept;

	int IndexOf(const ExactLocation::Coarse &) const noexcept;
	ExactLocation::Coarse PositionOf(int) const noexcept;

	/// returns nullptr if given position is out of range or the chunk
	/// is not loaded, so also works as a "has" function
	Chunk *Get(const ExactLocation::Coarse &) noexcept;
	const Chunk *Get(const ExactLocation::Coarse &) const noexcept;
	Chunk *operator [](int i) noexcept { return chunks[i]; }
	const Chunk *operator [](int i) const noexcept { return chunks[i]; }

	Chunk *RandomChunk(GaloisLFSR &rand) {
		return rand.From(chunks);
	}
	BlockLookup RandomBlock(GaloisLFSR &rand) {
		return BlockLookup(RandomChunk(rand), Chunk::ToPos(rand.Next<unsigned int>() % Chunk::size));
	}

	int Extent() const noexcept { return extent; }

	// raw iteration access, may contain nullptrs
	std::vector<Chunk *>::const_iterator begin() const noexcept { return chunks.begin(); }
	std::vector<Chunk *>::const_iterator end() const noexcept { return chunks.end(); }

	ExactLocation::Coarse CoordsBegin() const noexcept { return base - ExactLocation::Coarse(extent); }
	ExactLocation::Coarse CoordsEnd() const noexcept { return base + ExactLocation::Coarse(extent + 1); }

	void Register(Chunk &) noexcept;

	int TotalChunks() const noexcept { return total_length; }
	int IndexedChunks() const noexcept { return total_indexed; }
	int MissingChunks() const noexcept { return total_length - total_indexed; }

	ExactLocation::Coarse NextMissing() noexcept;

	const ExactLocation::Coarse &Base() const noexcept { return base; }
	void Rebase(const ExactLocation::Coarse &);

private:
	int GetCol(int) const noexcept;

	void Shift(Block::Face);

	void Clear() noexcept;
	void Scan() noexcept;

	void Set(int index, Chunk &) noexcept;
	void Unset(int index) noexcept;

private:
	ChunkStore &store;
	ExactLocation::Coarse base;
	int extent;
	int side_length;
	int total_length;
	int total_indexed;
	int last_missing;
	glm::ivec3 stride;
	std::vector<Chunk *> chunks;

};

}

#endif
