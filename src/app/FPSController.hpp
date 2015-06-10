#ifndef BLANK_APP_FPSCONTROLLER_HPP_
#define BLANK_APP_FPSCONTROLLER_HPP_

#include "../model/geometry.hpp"
#include "../world/Entity.hpp"

#include <glm/glm.hpp>


namespace blank {

class FPSController {

public:
	explicit FPSController(Entity &) noexcept;

	Ray Aim() const noexcept { return entity.Aim(entity.ChunkCoords()); }

	const glm::vec3 &Velocity() const noexcept { return velocity; }
	void Velocity(const glm::vec3 &vel) noexcept { velocity = vel; }

	// all angles in radians (full circle = 2π)
	float Pitch() const noexcept { return pitch; }
	void Pitch(float p) noexcept;
	void RotatePitch(float delta) noexcept;
	float Yaw() const noexcept { return yaw; }
	void Yaw(float y) noexcept;
	void RotateYaw(float delta) noexcept;

	void Update(int dt) noexcept;

private:
	Entity &entity;

	glm::vec3 velocity;

	float pitch;
	float yaw;

};

}

#endif
