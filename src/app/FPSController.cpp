#include "FPSController.hpp"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/rotate_vector.hpp>


namespace blank {

FPSController::FPSController(Entity &entity) noexcept
: entity(entity)
, pitch(0)
, yaw(0) {

}


void FPSController::Pitch(float p) noexcept {
	pitch = p;
	if (pitch > PI / 2) {
		pitch = PI / 2;
	} else if (pitch < -PI / 2) {
		pitch = -PI / 2;
	}
}

void FPSController::RotatePitch(float delta) noexcept {
	Pitch(pitch + delta);
}

void FPSController::Yaw(float y) noexcept {
	yaw = y;
	if (yaw > PI) {
		yaw -= PI * 2;
	} else if (yaw < -PI) {
		yaw += PI * 2;
	}
}

void FPSController::RotateYaw(float delta) noexcept {
	Yaw(yaw + delta);
}


void FPSController::Update(int dt) noexcept {
	entity.Orientation(glm::quat(glm::vec3(pitch, yaw, 0.0f)));
	entity.Velocity(glm::rotateY(velocity, yaw));
}

}
