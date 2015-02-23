#include "camera.hpp"

#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>


namespace blank {

Camera::Camera()
: FPSController()
, fov(45.0f)
, aspect(1.0f)
, near_clip(0.1f)
, far_clip(100.0f)
, projection(glm::perspective(fov, aspect, near_clip, far_clip))
, view(glm::inverse(Transform()))
, vp(projection) {

}


void Camera::Viewport(int width, int height) {
	Viewport(0, 0, width, height);
}

void Camera::Viewport(int x, int y, int width, int height) {
	glViewport(x, y, width, height);
	Aspect(width, height);
}

void Camera::FOV(float f) {
	fov = f;
	UpdateProjection();
}

void Camera::Aspect(float r) {
	aspect = r;
	UpdateProjection();
}

void Camera::Aspect(float w, float h) {
	Aspect(w / h);
}

void Camera::Clip(float near, float far) {
	near_clip = near;
	far_clip = far;
	UpdateProjection();
}

Ray Camera::Aim() const {
	const glm::mat4 inv_vp(glm::inverse(vp));
	glm::vec4 from = inv_vp * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);
	from /= from.w;
	glm::vec4 to = inv_vp * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	to /= to.w;
	return Ray{ glm::vec3(from), glm::normalize(glm::vec3(to - from)) };
}


void Camera::Update(int dt) {
	FPSController::Update(dt);
	view = glm::inverse(Transform());
	vp = projection * view;
}

void Camera::UpdateProjection() {
	projection = glm::perspective(fov, aspect, near_clip, far_clip);
}

}
