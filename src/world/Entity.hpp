#ifndef BLANK_WORLD_ENTITY_HPP_
#define BLANK_WORLD_ENTITY_HPP_

#include "Block.hpp"
#include "Chunk.hpp"
#include "../model/Model.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


namespace blank {

class Ray;
class Shape;

class Entity {

public:
	Entity() noexcept;

	bool HasShape() const noexcept { return shape; }
	const Shape *GetShape() const noexcept { return shape; }
	void SetShape(const Shape *, const glm::vec3 &color);
	void SetShapeless() noexcept;

	const glm::vec3 &Velocity() const noexcept { return velocity; }
	void Velocity(const glm::vec3 &) noexcept;

	const Block::Pos &Position() const noexcept { return position; }
	void Position(const Block::Pos &) noexcept;
	void Move(const glm::vec3 &delta) noexcept;

	const Chunk::Pos ChunkCoords() const noexcept { return chunk; }

	const glm::quat &AngularVelocity() const noexcept { return angular_velocity; }
	void AngularVelocity(const glm::quat &) noexcept;

	const glm::mat4 &Rotation() const noexcept { return rotation; }
	void Rotation(const glm::mat4 &) noexcept;
	void Rotate(const glm::quat &delta) noexcept;

	glm::mat4 Transform(const Chunk::Pos &chunk_offset) const noexcept;
	Ray Aim(const Chunk::Pos &chunk_offset) const noexcept;

	void Update(int dt) noexcept;

	void Draw() noexcept;

private:
	const Shape *shape;
	Model model;

	glm::vec3 velocity;
	Block::Pos position;
	Chunk::Pos chunk;

	glm::quat angular_velocity;
	glm::mat4 rotation;

};

}

#endif
