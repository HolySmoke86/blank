#ifndef BLANK_APP_FPSCONTROLLER_HPP_
#define BLANK_APP_FPSCONTROLLER_HPP_

#include "../model/geometry.hpp"
#include "../world/Entity.hpp"

#include <glm/glm.hpp>


namespace blank {

/// Sets entity rotation and velocity according to stored velocity
/// and pitch/yaw components.
/// Rotation is applied in yaw,pitch order (YX). Velocity is relative
/// to yaw only (Y axis).
class FPSController {

public:
	explicit FPSController(Entity &) noexcept;
	~FPSController();

	Entity &Controlled() noexcept { return entity; }
	const Entity &Controlled() const noexcept { return entity; }

	/// get position and face direction of controlled entity
	Ray Aim() const noexcept { return entity.Aim(entity.ChunkCoords()); }

	/// velocity, relative to heading (yaw only)
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
