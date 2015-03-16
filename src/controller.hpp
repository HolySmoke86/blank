#ifndef BLANK_CONTROLLER_HPP_
#define BLANK_CONTROLLER_HPP_

#include "entity.hpp"
#include "geometry.hpp"

#include <glm/glm.hpp>


namespace blank {

class FPSController {

public:
	explicit FPSController(Entity &);

	Ray Aim() const { return entity.Aim(entity.ChunkCoords()); }

	const glm::vec3 &Velocity() const { return velocity; }
	void Velocity(const glm::vec3 &vel) { velocity = vel; }

	// all angles in radians (full circle = 2π)
	float Pitch() const { return pitch; }
	void Pitch(float p);
	void RotatePitch(float delta);
	float Yaw() const { return yaw; }
	void Yaw(float y);
	void RotateYaw(float delta);

	void Update(int dt);

private:
	Entity &entity;

	glm::vec3 velocity;

	float pitch;
	float yaw;

};


class RandomWalk {

public:
	explicit RandomWalk(Entity &);

	void Update(int dt);

private:
	Entity &entity;

	int time_left;

};

}

#endif
