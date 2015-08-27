#ifndef BLANK_WORLD_ENTITY_HPP_
#define BLANK_WORLD_ENTITY_HPP_

#include "Chunk.hpp"
#include "../model/CompositeModel.hpp"
#include "../model/geometry.hpp"

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


namespace blank {

class DirectionalLighting;
class Shape;

class Entity {

public:
	Entity() noexcept;

	CompositeModel &GetModel() noexcept { return model; }
	const CompositeModel &GetModel() const noexcept { return model; }

	const std::string &Name() const noexcept { return name; }
	void Name(const std::string &n) { name = n; }

	const AABB &Bounds() const noexcept { return bounds; }
	void Bounds(const AABB &b) noexcept { bounds = b; }

	bool WorldCollidable() const noexcept { return world_collision; }
	void WorldCollidable(bool b) noexcept { world_collision = b; }

	const glm::vec3 &Velocity() const noexcept { return velocity; }
	void Velocity(const glm::vec3 &v) noexcept { velocity = v; }

	const glm::vec3 &Position() const noexcept { return model.Position(); }
	void Position(const Chunk::Pos &, const glm::vec3 &) noexcept;
	void Position(const glm::vec3 &) noexcept;
	void Move(const glm::vec3 &delta) noexcept;

	const Chunk::Pos ChunkCoords() const noexcept { return chunk; }

	glm::vec3 AbsolutePosition() const noexcept {
		return glm::vec3(chunk * Chunk::Extent()) + Position();
	}
	glm::vec3 AbsoluteDifference(const Entity &other) const noexcept {
		return glm::vec3((chunk - other.chunk) * Chunk::Extent()) + Position() - other.Position();
	}

	/// direction is rotation axis, magnitude is speed in rad/ms
	const glm::vec3 &AngularVelocity() const noexcept { return angular_velocity; }
	void AngularVelocity(const glm::vec3 &v) noexcept { angular_velocity = v; }

	const glm::quat &Orientation() const noexcept { return model.Orientation(); }
	void Orientation(const glm::quat &o) noexcept { model.Orientation(o); }
	void Rotate(const glm::quat &delta) noexcept;

	glm::mat4 ChunkTransform(const Chunk::Pos &chunk_offset) const noexcept;
	glm::mat4 Transform(const Chunk::Pos &chunk_offset) const noexcept;
	Ray Aim(const Chunk::Pos &chunk_offset) const noexcept;

	void Ref() noexcept { ++ref_count; }
	void UnRef() noexcept { --ref_count; }
	void Kill() noexcept { dead = true; }
	bool Referenced() const noexcept { return ref_count > 0; }
	bool Dead() const noexcept { return dead; }
	bool CanRemove() const noexcept { return dead && ref_count <= 0; }

	void Update(int dt) noexcept;

	void Render(const glm::mat4 &M, DirectionalLighting &prog) noexcept {
		model.Render(M, prog);
	}

private:
	CompositeModel model;

	std::string name;

	AABB bounds;

	glm::vec3 velocity;
	Chunk::Pos chunk;

	glm::vec3 angular_velocity;

	int ref_count;

	bool world_collision;
	bool dead;

};

}

#endif
