#include "controller.hpp"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/rotate_vector.hpp>


namespace blank {

FPSController::FPSController(Entity &entity)
: entity(entity)
, pitch(0)
, yaw(0) {

}


void FPSController::Pitch(float p) {
	pitch = p;
	if (pitch > PI / 2) {
		pitch = PI / 2;
	} else if (pitch < -PI / 2) {
		pitch = -PI / 2;
	}
}

void FPSController::RotatePitch(float delta) {
	Pitch(pitch + delta);
}

void FPSController::Yaw(float y) {
	yaw = y;
	if (yaw > PI) {
		yaw -= PI * 2;
	} else if (yaw < -PI) {
		yaw += PI * 2;
	}
}

void FPSController::RotateYaw(float delta) {
	Yaw(yaw + delta);
}


void FPSController::Update(int dt) {
	entity.Rotation(glm::eulerAngleYX(yaw, pitch));
	entity.Velocity(glm::rotateY(velocity, yaw));
}


RandomWalk::RandomWalk(Entity &e)
: entity(e)
, time_left(0) {

}


void RandomWalk::Update(int dt) {
	time_left -= dt;
	if (time_left > 0) return;
	time_left += 2500 + (rand() % 5000);

	constexpr float move_vel = 0.0005f;

	glm::vec3 new_vel = entity.Velocity();

	switch (rand() % 9) {
		case 0:
			new_vel.x = -move_vel;
			break;
		case 1:
			new_vel.x = 0.0f;
			break;
		case 2:
			new_vel.x = move_vel;
			break;
		case 3:
			new_vel.y = -move_vel;
			break;
		case 4:
			new_vel.y = 0.0f;
			break;
		case 5:
			new_vel.y = move_vel;
			break;
		case 6:
			new_vel.z = -move_vel;
			break;
		case 7:
			new_vel.z = 0.0f;
			break;
		case 8:
			new_vel.z = move_vel;
			break;
	}

	entity.Velocity(new_vel);
}

}
