#ifndef BLANK_WORLD_ENTITYSTATE_HPP_
#define BLANK_WORLD_ENTITYSTATE_HPP_

#include "Chunk.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


namespace blank {

struct EntityState {

	glm::ivec3 chunk_pos;
	glm::vec3 block_pos;
	glm::vec3 velocity;

	glm::quat orient;
	glm::vec3 ang_vel;

	EntityState();

	/// make sure block_pos is within chunk bounds
	void AdjustPosition() noexcept;

	/// do an integration step of dt milliseconds
	void Update(int dt) noexcept;

	/// get a position vector relative to the (0,0,0) chunk
	glm::vec3 AbsolutePosition() const noexcept {
		return glm::vec3(chunk_pos * Chunk::Extent()) + block_pos;
	}
	/// get a position vector relative to given reference chunk
	glm::vec3 RelativePosition(const glm::ivec3 &reference) const noexcept {
		return glm::vec3((chunk_pos - reference) * Chunk::Extent()) + block_pos;
	}

	/// get the difference between this and the given position
	glm::vec3 Diff(const EntityState &other) const noexcept {
		return RelativePosition(other.chunk_pos) - other.block_pos;
	}

	/// get entity state as a matrix tranform relative to given reference chunk
	glm::mat4 Transform(const glm::ivec3 &reference) const noexcept;

};

}

#endif
