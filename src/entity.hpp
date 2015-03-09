#ifndef BLANK_ENTITY_HPP_
#define BLANK_ENTITY_HPP_

#include "block.hpp"
#include "chunk.hpp"
#include "geometry.hpp"

#include <glm/glm.hpp>


namespace blank {

class Entity {

public:
	Entity();

	const glm::vec3 &Velocity() const { return velocity; }
	void Velocity(const glm::vec3 &);

	const Block::Pos &Position() const { return position; }
	void Position(const Block::Pos &);
	void Move(const glm::vec3 &delta);

	const Chunk::Pos ChunkCoords() const { return chunk; }

	const glm::mat4 &Rotation() const { return rotation; }
	void Rotation(const glm::mat4 &);

	glm::mat4 Transform(const Chunk::Pos &chunk_offset) const;
	Ray Aim(const Chunk::Pos &chunk_offset) const;

	void Update(int dt);

private:
	glm::vec3 velocity;
	Block::Pos position;
	Chunk::Pos chunk;

	glm::mat4 rotation;

};

}

#endif
