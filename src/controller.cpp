#include "controller.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>


namespace blank {

FPSController::FPSController()
: velocity(0, 0, 0)
, position(0, 0, 0)
, pitch(0)
, yaw(0) {

}


glm::mat4 FPSController::Transform() const {
	return glm::translate(position) * glm::eulerAngleYX(yaw, pitch);
}


void FPSController::OrientationVelocity(const glm::vec3 &vel) {
	velocity = glm::rotateY(vel, yaw);
}


void FPSController::Update(int dt) {
	position += velocity * float(dt);
}

}
