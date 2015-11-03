#ifndef BLANK_WORLD_ENTITYSTATE_HPP_
#define BLANK_WORLD_ENTITYSTATE_HPP_

#include "../geometry/Location.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


namespace blank {

struct EntityState {

	ExactLocation pos;
	glm::vec3 velocity;

	glm::quat orient;
	float pitch;
	float yaw;

	EntityState();

	/// make sure pos.block is within chunk bounds
	void AdjustPosition() noexcept;
	/// make sure pitch and yaw are normalized
	void AdjustHeading() noexcept;

	/// get a position vector relative to the (0,0,0) chunk
	glm::vec3 AbsolutePosition() const noexcept {
		return pos.Absolute();
	}
	/// get a position vector relative to given reference chunk
	glm::vec3 RelativePosition(const glm::ivec3 &reference) const noexcept {
		return pos.Relative(reference).Absolute();
	}

	/// get the difference between this and the given position
	glm::vec3 Diff(const EntityState &other) const noexcept {
		return pos.Difference(other.pos).Absolute();
	}

	/// get entity state as a matrix tranform relative to given reference chunk
	glm::mat4 Transform(const glm::ivec3 &reference) const noexcept;

};

}

#endif
