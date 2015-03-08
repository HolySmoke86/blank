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
, yaw(0)
, transform(1.0f)
, dirty(true)
, move_velocity(0.003f)
, pitch_sensitivity(-0.0025f)
, yaw_sensitivity(-0.001f)
, front(false)
, back(false)
, left(false)
, right(false)
, up(false)
, down(false) {

}


const glm::mat4 &FPSController::Transform() const {
	if (dirty) {
		transform = glm::translate(position) * glm::eulerAngleYX(yaw, pitch);
		dirty = false;
	}
	return transform;
}

Ray FPSController::Aim() const {
	glm::vec4 from = Transform() * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	from /= from.w;
	glm::vec4 to = Transform() * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);
	to /= to.w;
	return Ray{ glm::vec3(from), glm::normalize(glm::vec3(to - from)) };
}


void FPSController::OrientationVelocity(const glm::vec3 &vel) {
	Velocity(glm::rotateY(vel, yaw));
}


void FPSController::Pitch(float p) {
	pitch = p;
	if (pitch > PI / 2) {
		pitch = PI / 2;
	} else if (pitch < -PI / 2) {
		pitch = -PI / 2;
	}
	dirty = true;
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
	dirty = true;
}

void FPSController::RotateYaw(float delta) {
	Yaw(yaw + delta);
}


void FPSController::HandleKeyboard(const SDL_KeyboardEvent &event) {
	switch (event.keysym.sym) {
		case SDLK_w:
			front = event.state == SDL_PRESSED;
			break;
		case SDLK_s:
			back = event.state == SDL_PRESSED;
			break;
		case SDLK_a:
			left = event.state == SDL_PRESSED;
			break;
		case SDLK_d:
			right = event.state == SDL_PRESSED;
			break;
		case SDLK_q:
		case SDLK_SPACE:
			up = event.state == SDL_PRESSED;
			break;
		case SDLK_e:
		case SDLK_LSHIFT:
			down = event.state == SDL_PRESSED;
			break;
	}
}

void FPSController::HandleMouse(const SDL_MouseMotionEvent &event) {
	RotateYaw(event.xrel * yaw_sensitivity);
	RotatePitch(event.yrel * pitch_sensitivity);
}


void FPSController::Update(int dt) {
	glm::vec3 vel;
	if (right && !left) {
		vel.x = move_velocity;
	} else if (left && !right) {
		vel.x = -move_velocity;
	}
	if (up && !down) {
		vel.y = move_velocity;
	} else if (down && !up) {
		vel.y = -move_velocity;
	}
	if (back && !front) {
		vel.z = move_velocity;
	} else if (front && !back) {
		vel.z = -move_velocity;
	}
	OrientationVelocity(vel);

	Move(velocity * float(dt));
}

}
