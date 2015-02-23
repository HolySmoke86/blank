#ifndef BLANK_CONTROLLER_HPP_
#define BLANK_CONTROLLER_HPP_

#include <glm/glm.hpp>


namespace blank {

class FPSController {

public:
	FPSController();

	glm::mat4 Transform() const;

	void Velocity(glm::vec3 vel) { velocity = vel; }
	void OrientationVelocity(const glm::vec3 &vel);
	void Position(glm::vec3 pos) { position = pos; }
	void Move(glm::vec3 delta) { position += delta; }

	// all angles in radians (full circle = 2π)
	float Pitch() const { return pitch; }
	void Pitch(float p) { pitch = p; }
	void RotatePitch(float delta) { pitch += delta; }
	float Yaw() const { return yaw; }
	void Yaw(float y) { yaw = y; }
	void RotateYaw(float delta) { yaw += delta; }

	void Update(int dt);

private:
	glm::vec3 velocity;
	glm::vec3 position;
	float pitch;
	float yaw;

};

}

#endif
