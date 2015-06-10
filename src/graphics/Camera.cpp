#include "Camera.hpp"

#include "../model/geometry.hpp"

#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>


namespace blank {

Camera::Camera() noexcept
: fov(PI_0p25)
, aspect(1.0f)
, near_clip(0.1f)
, far_clip(256.0f)
, projection(glm::perspective(fov, aspect, near_clip, far_clip)) {

}


void Camera::Viewport(int width, int height) noexcept {
	Viewport(0, 0, width, height);
}

void Camera::Viewport(int x, int y, int width, int height) noexcept {
	glViewport(x, y, width, height);
	Aspect(width, height);
}

void Camera::FOV(float f) noexcept {
	fov = f;
	UpdateProjection();
}

void Camera::Aspect(float r) noexcept {
	aspect = r;
	UpdateProjection();
}

void Camera::Aspect(float w, float h) noexcept {
	Aspect(w / h);
}

void Camera::Clip(float near, float far) noexcept {
	near_clip = near;
	far_clip = far;
	UpdateProjection();
}


void Camera::UpdateProjection() noexcept {
	projection = glm::perspective(fov, aspect, near_clip, far_clip);
}

}
