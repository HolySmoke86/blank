#ifndef BLANK_WORLD_WORLDCOLLISION_HPP_
#define BLANK_WORLD_WORLDCOLLISION_HPP_

#include "BlockType.hpp"
#include "Chunk.hpp"

#include <glm/glm.hpp>


namespace blank {

struct WorldCollision {

	Chunk *chunk;
	int block;

	float depth;
	glm::vec3 normal;

	WorldCollision()
	: chunk(nullptr), block(-1), depth(0.0f), normal(0.0f) { }
	WorldCollision(Chunk *c, int b, float d, const glm::vec3 &n)
	: chunk(c), block(b), depth(d), normal(n) { }

	/// check if an actual collision
	operator bool() const noexcept { return chunk; }

	// following only valid if test true
	Chunk &GetChunk() noexcept { return *chunk; }
	const Chunk &GetChunk() const noexcept { return *chunk; }
	const Block &GetBlock() const noexcept { return GetChunk().BlockAt(block); }
	const BlockType &GetType() const noexcept { return GetChunk().Type(GetBlock()); }

	void SetBlock(const Block &b) noexcept { GetChunk().SetBlock(block, b); }

	bool Blocks() const noexcept { return chunk->Type(block).collide_block; }

	const Chunk::Pos &ChunkPos() const noexcept { return GetChunk().Position(); }

	glm::ivec3 BlockPos() const noexcept { return Chunk::ToPos(block); }
	glm::vec3 BlockCoords() const noexcept { return Chunk::ToCoords(block); }
	glm::mat4 BlockTransform() const noexcept { return GetChunk().ToTransform(BlockPos(), block); }

};

}

#endif
