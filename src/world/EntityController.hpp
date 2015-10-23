#ifndef BLANK_WORLD_ENTITYCONTROLLER_HPP_
#define BLANK_WORLD_ENTITYCONTROLLER_HPP_

#include "EntityState.hpp"

#include <glm/glm.hpp>


namespace blank {

class Entity;

struct EntityController {

	virtual ~EntityController();

	virtual void Update(Entity &, float dt) = 0;

	virtual glm::vec3 ControlForce(const Entity &, const EntityState &) const = 0;


	/// try to add as much of add to out so it doesn't exceed max
	/// returns true if it's maxed out
	static bool MaxOutForce(
		glm::vec3 &out,
		const glm::vec3 &add,
		float max
	) noexcept;
	/// give a force that makes state's velocity converge with given target velocity
	/// over 1/n seconds
	static inline glm::vec3 TargetVelocity(
		const glm::vec3 &target,
		const EntityState &state,
		float n
	) noexcept {
		return (target - state.velocity) * n;
	}
	/// give a force that makes state come to a halt over 1/n seconds
	static inline glm::vec3 Halt(
		const EntityState &state,
		float n
	) noexcept {
		return state.velocity * -n;
	}

};

}

#endif
