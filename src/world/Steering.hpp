#ifndef BLANK_WORLD_STEERING_HPP
#define BLANK_WORLD_STEERING_HPP

#include "../geometry/Location.hpp"
#include "../geometry/primitive.hpp"

#include <glm/glm.hpp>


namespace blank {

class Entity;
class EntityState;
class World;

class Steering {

public:
	enum Behaviour {
		// standalone behaviours
		HALT = 0x0001,
		WANDER = 0x0002,
		OBSTACLE_AVOIDANCE = 0x0004,

		// requires target velocity
		TARGET_VELOCITY = 0x0008,

		// behaviours requiring a target entity
		EVADE_TARGET = 0x0010,
		PURSUE_TARGET = 0x0020,
	};

	explicit Steering(const Entity &);
	~Steering();

	Steering &Enable(unsigned int b) noexcept { enabled |= b; return *this; }
	Steering &Disable(unsigned int b) noexcept { enabled &= ~b; return *this; }
	bool AnyEnabled(unsigned int b) const noexcept { return enabled & b; }
	bool AllEnabled(unsigned int b) const noexcept { return (enabled & b) == b; }

	Steering &SetTargetEntity(Entity &) noexcept;
	Steering &ClearTargetEntity() noexcept;
	bool HasTargetEntity() const noexcept { return target_entity; }
	const Entity &GetTargetEntity() const noexcept { return *target_entity; }

	Steering &SetTargetVelocity(const glm::vec3 &v) noexcept { target_velocity = v; return *this; }
	const glm::vec3 &GetTargetVelocity() const noexcept { return target_velocity; }

	/// time in seconds in which steering tried to arrive at target velocity
	Steering &SetAcceleration(float a) noexcept { accel = a; return *this; }
	/// maximum magnitude of velocity that behaviours generate
	Steering &SetSpeed(float s) noexcept { speed = s; return *this; }

	/// configure wandering
	/// r is the radius of the sphere
	/// dist is the distance between the entity and the sphere's center
	/// disp is the maximum variance of the point on the sphere in units per second
	Steering &SetWanderParams(float r, float dist, float disp) noexcept {
		wander_radius = r;
		wander_dist = dist;
		wander_disp = disp;
		return *this;
	}

	void Update(World &, float dt);

	glm::vec3 Force(const EntityState &) const noexcept;

private:
	void UpdateWander(World &, float dt);
	void UpdateObstacle(World &);

	/// try to add as much of in to out so it doesn't exceed max
	/// returns true if it's maxed out
	static bool SumForce(glm::vec3 &out, const glm::vec3 &in, float max) noexcept;

	/// slow down to a halt
	glm::vec3 Halt(const EntityState &) const noexcept;
	/// accelerate to match given velocity
	glm::vec3 TargetVelocity(const EntityState &, const glm::vec3 &) const noexcept;
	/// move towards given location
	glm::vec3 Seek(const EntityState &, const ExactLocation &) const noexcept;
	/// move away from given location
	glm::vec3 Flee(const EntityState &, const ExactLocation &) const noexcept;
	/// try to halt at given location
	glm::vec3 Arrive(const EntityState &, const ExactLocation &) const noexcept;
	/// seek given entity's predicted position
	glm::vec3 Pursuit(const EntityState &, const Entity &) const noexcept;
	/// flee given entity's predicted position
	glm::vec3 Evade(const EntityState &, const Entity &) const noexcept;
	/// move around for no reason
	glm::vec3 Wander(const EntityState &) const noexcept;
	/// try not to crash into blocks
	glm::vec3 ObstacleAvoidance(const EntityState &) const noexcept;

private:
	const Entity &entity;

	Entity *target_entity;
	glm::vec3 target_velocity;

	float accel;
	float speed;

	float wander_radius;
	float wander_dist;
	float wander_disp;
	glm::vec3 wander_pos;

	glm::vec3 obstacle_dir;

	unsigned int enabled;



};

}

#endif
