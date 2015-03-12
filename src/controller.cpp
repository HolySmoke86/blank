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

}
