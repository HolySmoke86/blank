#ifndef BLANK_WORLD_BLOCKLOOKUP_HPP_
#define BLANK_WORLD_BLOCKLOOKUP_HPP_

#include "Block.hpp"
#include "Chunk.hpp"


namespace blank {

class BlockLookup {

public:
	/// resolve chunk/position from oob coordinates
	BlockLookup(Chunk *c, const RoughLocation::Fine &p) noexcept;

	/// resolve chunk/position from ib coordinates and direction
	BlockLookup(Chunk *c, const RoughLocation::Fine &p, Block::Face dir) noexcept;

	/// check if lookup was successful
	operator bool() const { return chunk; }

	// only valid if lookup was successful
	Chunk &GetChunk() const noexcept { return *chunk; }
	const RoughLocation::Fine &GetBlockPos() const noexcept { return pos; }
	int GetBlockIndex() const noexcept { return Chunk::ToIndex(pos); }
	ExactLocation::Fine GetBlockCoords() const noexcept { return Chunk::ToCoords(pos); }
	const Block &GetBlock() const noexcept { return GetChunk().BlockAt(GetBlockPos()); }
	const BlockType &GetType() const noexcept { return GetChunk().Type(GetBlock()); }
	int GetLight() const noexcept { return GetChunk().GetLight(GetBlockPos()); }

	void SetBlock(const Block &b) noexcept { GetChunk().SetBlock(GetBlockPos(), b); }

	// traverse in given direction
	BlockLookup Next(Block::Face f) const { return BlockLookup(chunk, pos, f); }

private:
	Chunk *chunk;
	RoughLocation::Fine pos;

};

}

#endif
