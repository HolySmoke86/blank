#ifndef BLANK_WORLD_ENTITY_HPP_
#define BLANK_WORLD_ENTITY_HPP_

#include "Block.hpp"
#include "Chunk.hpp"
#include "../model/geometry.hpp"
#include "../model/EntityModel.hpp"

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


namespace blank {

class Shape;

class Entity {

public:
	Entity() noexcept;

	bool HasShape() const noexcept { return shape; }
	const Shape *GetShape() const noexcept { return shape; }
	void SetShape(const Shape *, const glm::vec3 &color);
	void SetShapeless() noexcept;

	const std::string &Name() const noexcept { return name; }
	void Name(const std::string &n) { name = n; }

	const AABB &Bounds() const noexcept { return bounds; }
	void Bounds(const AABB &b) noexcept { bounds = b; }

	bool WorldCollidable() const noexcept { return world_collision; }
	void WorldCollidable(bool b) noexcept { world_collision = b; }

	const glm::vec3 &Velocity() const noexcept { return velocity; }
	void Velocity(const glm::vec3 &) noexcept;

	const Block::Pos &Position() const noexcept { return position; }
	void Position(const Chunk::Pos &, const Block::Pos &) noexcept;
	void Position(const Block::Pos &) noexcept;
	void Move(const glm::vec3 &delta) noexcept;

	const Chunk::Pos ChunkCoords() const noexcept { return chunk; }

	glm::vec3 AbsolutePosition() const noexcept {
		return glm::vec3(chunk * Chunk::Extent()) + position;
	}
	glm::vec3 AbsoluteDifference(const Entity &other) const noexcept {
		return glm::vec3((chunk - other.chunk) * Chunk::Extent()) + position - other.position;
	}

	/// direction is rotation axis, magnitude is speed in rad/ms
	const glm::vec3 &AngularVelocity() const noexcept { return angular_velocity; }
	void AngularVelocity(const glm::vec3 &) noexcept;

	const glm::quat &Rotation() const noexcept { return rotation; }
	void Rotation(const glm::quat &) noexcept;
	void Rotate(const glm::quat &delta) noexcept;

	glm::mat4 Transform(const Chunk::Pos &chunk_offset) const noexcept;
	Ray Aim(const Chunk::Pos &chunk_offset) const noexcept;

	void Remove() noexcept { remove = true; }
	bool CanRemove() const noexcept { return remove; }

	void Update(int dt) noexcept;

	void Draw() noexcept {
		model.Draw();
	}

private:
	const Shape *shape;
	EntityModel model;

	std::string name;

	AABB bounds;

	glm::vec3 velocity;
	Block::Pos position;
	Chunk::Pos chunk;

	glm::vec3 angular_velocity;
	glm::quat rotation;

	bool world_collision;
	bool remove;

};

}

#endif
