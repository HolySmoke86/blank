#ifndef BLANK_WORLD_ENTITY_HPP_
#define BLANK_WORLD_ENTITY_HPP_

#include "Chunk.hpp"
#include "EntityState.hpp"
#include "../geometry/primitive.hpp"
#include "../model/Instance.hpp"

#include <cstdint>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


namespace blank {

class DirectionalLighting;
class EntityController;
class Shape;

class Entity {

public:
	Entity() noexcept;
	~Entity() noexcept;

	// note that when copying an entity which owns its controller, the
	// original must outlive the copy, otherwise the copy ends up with
	// an invalid controller pointer
	Entity(const Entity &) noexcept;
	Entity &operator =(const Entity &) = delete;

	bool HasController() const noexcept { return ctrl; }
	// entity takes over ownership of controller
	void SetController(EntityController *c) noexcept;
	// entity uses shared controller
	void SetController(EntityController &c) noexcept;
	void UnsetController() noexcept;
	EntityController &GetController() noexcept { return *ctrl; }
	const EntityController &GetController() const noexcept { return *ctrl; }

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

	float MaxVelocity() const noexcept { return max_vel; }
	void MaxVelocity(float v) noexcept { max_vel = v; }
	float MaxControlForce() const noexcept { return max_force; }
	void MaxControlForce(float f) noexcept { max_force = f; }

	glm::vec3 ControlForce(const EntityState &) const noexcept;

	const glm::vec3 &Velocity() const noexcept { return state.velocity; }

	const ExactLocation::Fine &Position() const noexcept { return state.pos.block; }
	void Position(const ExactLocation::Coarse &, const ExactLocation::Fine &) noexcept;
	void Position(const ExactLocation::Fine &) noexcept;

	const glm::ivec3 ChunkCoords() const noexcept { return state.pos.chunk; }

	glm::vec3 AbsolutePosition() const noexcept {
		return state.AbsolutePosition();
	}
	glm::vec3 AbsoluteDifference(const Entity &other) const noexcept {
		return state.Diff(other.state);
	}

	/// orientation of local coordinate system
	const glm::quat &Orientation() const noexcept { return state.orient; }

	/// orientation of head within local coordinate system, in radians
	float Pitch() const noexcept { return state.pitch; }
	float Yaw() const noexcept { return state.yaw; }
	void TurnHead(float delta_pitch, float delta_yaw) noexcept;
	void SetHead(float pitch, float yaw) noexcept;

	/// get a transform for this entity's coordinate space
	const glm::mat4 Transform() const noexcept { return model_transform; }
	/// get a transform for this entity's coordinate space relative to reference chunk
	glm::mat4 Transform(const glm::ivec3 &reference) const noexcept;
	/// get a transform for this entity's view space relative to reference chunk
	glm::mat4 ViewTransform(const glm::ivec3 &reference) const noexcept;
	/// get a ray in entity's face direction originating from center of vision
	Ray Aim(const ExactLocation::Coarse &chunk_offset) const noexcept;

	/// true if this entity's position will change (significantly) the next update
	bool Moving() const noexcept { return speed > 0.0f; }
	/// magnitude of velocity
	float Speed() const noexcept { return speed; }
	/// normalized velocity or heading if standing still
	const glm::vec3 &Heading() const noexcept { return heading; }

	void SetState(const EntityState &s) noexcept { state = s; }
	const EntityState &GetState() const noexcept { return state; }

	void Ref() noexcept { ++ref_count; }
	void UnRef() noexcept { --ref_count; }
	void Kill() noexcept { dead = true; }
	bool Referenced() const noexcept { return ref_count > 0; }
	bool Dead() const noexcept { return dead; }
	bool CanRemove() const noexcept { return dead && ref_count <= 0; }

	void Update(float dt);

	void Render(const glm::mat4 &M, DirectionalLighting &prog) noexcept {
		if (model) model.Render(M, prog);
	}

private:
	void UpdateTransforms() noexcept;
	void UpdateHeading() noexcept;
	void UpdateModel(float dt) noexcept;
public:
	// temporarily made public so AI can use it until it's smoothed out to be suitable for players, too
	void OrientBody(float dt) noexcept;
private:
	void OrientHead(float dt) noexcept;

private:
	EntityController *ctrl;
	Instance model;

	std::uint32_t id;
	std::string name;

	AABB bounds;
	EntityState state;

	/// chunk to model space
	glm::mat4 model_transform;
	/// model to view space
	/// if this entity has no model, the eyes are assumed to
	/// be at origin and oriented towards pitch of model space
	glm::mat4 view_transform;
	float speed;
	glm::vec3 heading;

	// TODO: I'd prefer a drag solution
	float max_vel;
	float max_force;

	int ref_count;

	bool world_collision;
	bool dead;

	bool owns_controller;

};

}

#endif
