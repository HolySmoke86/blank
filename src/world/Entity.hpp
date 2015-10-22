#ifndef BLANK_WORLD_ENTITY_HPP_
#define BLANK_WORLD_ENTITY_HPP_

#include "Chunk.hpp"
#include "EntityState.hpp"
#include "../model/Instance.hpp"
#include "../model/geometry.hpp"

#include <cstdint>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


namespace blank {

class DirectionalLighting;
class Shape;

class Entity {

public:
	Entity() noexcept;

	Instance &GetModel() noexcept { return model; }
	const Instance &GetModel() const noexcept { return model; }

	std::uint32_t ID() const noexcept { return id; }
	void ID(std::uint32_t i) noexcept { id = i; }

	const std::string &Name() const noexcept { return name; }
	void Name(const std::string &n) { name = n; }

	const AABB &Bounds() const noexcept { return bounds; }
	void Bounds(const AABB &b) noexcept { bounds = b; }

	bool WorldCollidable() const noexcept { return world_collision; }
	void WorldCollidable(bool b) noexcept { world_collision = b; }

	/// desired velocity in local coordinate system
	const glm::vec3 &TargetVelocity() const noexcept { return tgt_vel; }
	void TargetVelocity(const glm::vec3 &v) noexcept { tgt_vel = v; }

	const glm::vec3 &Velocity() const noexcept { return state.velocity; }
	void Velocity(const glm::vec3 &v) noexcept { state.velocity = v; }

	const glm::vec3 &Position() const noexcept { return state.block_pos; }
	void Position(const glm::ivec3 &, const glm::vec3 &) noexcept;
	void Position(const glm::vec3 &) noexcept;

	const glm::ivec3 ChunkCoords() const noexcept { return state.chunk_pos; }

	glm::vec3 AbsolutePosition() const noexcept {
		return state.AbsolutePosition();
	}
	glm::vec3 AbsoluteDifference(const Entity &other) const noexcept {
		return state.Diff(other.state);
	}

	/// orientation of local coordinate system
	const glm::quat &Orientation() const noexcept { return state.orient; }
	void Orientation(const glm::quat &o) noexcept { state.orient = o; }

	/// orientation of head within local coordinate system, in radians
	float Pitch() const noexcept { return state.pitch; }
	float Yaw() const noexcept { return state.yaw; }
	void TurnHead(float delta_pitch, float delta_yaw) noexcept;
	void SetHead(float pitch, float yaw) noexcept;

	/// get a transform for this entity's coordinate space
	glm::mat4 Transform(const glm::ivec3 &reference) const noexcept;
	/// get a transform for this entity's view space
	glm::mat4 ViewTransform(const glm::ivec3 &reference) const noexcept;
	/// get a ray in entity's face direction originating from center of vision
	Ray Aim(const Chunk::Pos &chunk_offset) const noexcept;

	void SetState(const EntityState &s) noexcept { state = s; UpdateModel(); }
	const EntityState &GetState() const noexcept { return state; }

	void Ref() noexcept { ++ref_count; }
	void UnRef() noexcept { --ref_count; }
	void Kill() noexcept { dead = true; }
	bool Referenced() const noexcept { return ref_count > 0; }
	bool Dead() const noexcept { return dead; }
	bool CanRemove() const noexcept { return dead && ref_count <= 0; }

	void Render(const glm::mat4 &M, DirectionalLighting &prog) noexcept {
		if (model) model.Render(M, prog);
	}

private:
	void UpdateModel() noexcept;

private:
	Instance model;

	std::uint32_t id;
	std::string name;

	AABB bounds;
	EntityState state;
	glm::vec3 tgt_vel;

	int ref_count;

	bool world_collision;
	bool dead;

};

}

#endif
