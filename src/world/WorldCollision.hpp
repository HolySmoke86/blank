#ifndef BLANK_WORLD_WORLDCOLLISION_HPP_
#define BLANK_WORLD_WORLDCOLLISION_HPP_

#include <glm/glm.hpp>


namespace blank {

class Chunk;

struct WorldCollision {

	const Chunk *chunk;
	int block;

	float depth;
	glm::vec3 normal;

	WorldCollision(const Chunk *c, int b, float d, const glm::vec3 &n)
	: chunk(c), block(b), depth(d), normal(n) { }

};

}

#endif
