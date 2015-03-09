#ifndef BLANK_CONTROLLER_HPP_
#define BLANK_CONTROLLER_HPP_

#include "entity.hpp"
#include "geometry.hpp"

#include <SDL.h>
#include <glm/glm.hpp>


namespace blank {

class FPSController {

public:
	explicit FPSController(Entity &);

	Ray Aim() const { return entity.Aim(entity.ChunkCoords()); }

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
	Entity &entity;

	float pitch;
	float yaw;

	float move_velocity;
	float pitch_sensitivity;
	float yaw_sensitivity;

	bool front, back, left, right, up, down;

};

}

#endif
