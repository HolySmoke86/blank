#ifndef BLANK_WORLD_WORLDCOLLISION_HPP_
#define BLANK_WORLD_WORLDCOLLISION_HPP_

#include "BlockType.hpp"
#include "Chunk.hpp"

#include <glm/glm.hpp>


namespace blank {

struct WorldCollision {

	const Chunk *chunk;
	int block;

	float depth;
	glm::vec3 normal;

	WorldCollision(const Chunk *c, int b, float d, const glm::vec3 &n)
	: chunk(c), block(b), depth(d), normal(n) { }

	bool Blocks() const noexcept { return chunk->Type(block).collide_block; }

	glm::vec3 BlockCoords() const noexcept { return Chunk::ToCoords(block); }

};

}

#endif
