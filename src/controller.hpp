#ifndef BLANK_CONTROLLER_HPP_
#define BLANK_CONTROLLER_HPP_

#include "geometry.hpp"

#include <SDL.h>
#include <glm/glm.hpp>


namespace blank {

class FPSController {

public:
	FPSController();

	const glm::mat4 &Transform() const;
	Ray Aim() const;

	void Velocity(glm::vec3 vel) { velocity = vel; dirty = true; }
	void OrientationVelocity(const glm::vec3 &vel);
	void Position(glm::vec3 pos) { position = pos; dirty = true; }
	void Move(glm::vec3 delta) { Position(position + delta); }

	// all angles in radians (full circle = 2π)
	float Pitch() const { return pitch; }
	void Pitch(float p);
	void RotatePitch(float delta);
	float Yaw() const { return yaw; }
	void Yaw(float y);
	void RotateYaw(float delta);

	void HandleKeyboard(const SDL_KeyboardEvent &);
	void HandleMouse(const SDL_MouseMotionEvent &);

	void Update(int dt);

private:
	glm::vec3 velocity;
	glm::vec3 position;
	float pitch;
	float yaw;

	mutable glm::mat4 transform;
	mutable bool dirty;

	float move_velocity;
	float pitch_sensitivity;
	float yaw_sensitivity;

	bool front, back, left, right, up, down;

};

}

#endif
