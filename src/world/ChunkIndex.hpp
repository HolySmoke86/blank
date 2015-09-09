#ifndef BLANK_WORLD_CHUNKINDEX_HPP_
#define BLANK_WORLD_CHUNKINDEX_HPP_

#include "Chunk.hpp"

#include <vector>


namespace blank {

class ChunkStore;

class ChunkIndex {

public:
	ChunkIndex(ChunkStore &, const Chunk::Pos &base, int extent);
	~ChunkIndex();

	ChunkIndex(const ChunkIndex &) = delete;
	ChunkIndex &operator =(const ChunkIndex &) = delete;

public:
	bool InRange(const Chunk::Pos &) const noexcept;
	int IndexOf(const Chunk::Pos &) const noexcept;
	Chunk::Pos PositionOf(int) const noexcept;
	/// returns nullptr if given position is out of range or the chunk
	/// is not loaded, so also works as a "has" function
	Chunk *Get(const Chunk::Pos &) noexcept;
	const Chunk *Get(const Chunk::Pos &) const noexcept;
	Chunk *operator [](int i) noexcept { return chunks[i]; }
	const Chunk *operator [](int i) const noexcept { return chunks[i]; }

	void Register(Chunk &) noexcept;

	int TotalChunks() const noexcept { return total_length; }
	int IndexedChunks() const noexcept { return total_indexed; }
	int MissingChunks() const noexcept { return total_length - total_indexed; }

	Chunk::Pos NextMissing() noexcept;

	const Chunk::Pos &Base() const noexcept { return base; }
	void Rebase(const Chunk::Pos &);

private:
	int GetCol(int) const noexcept;

	void Shift(Block::Face);

	void Clear() noexcept;
	void Scan() noexcept;

	void Set(int index, Chunk &) noexcept;
	void Unset(int index) noexcept;

private:
	ChunkStore &store;
	Chunk::Pos base;
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
