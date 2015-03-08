#include "camera.hpp"

#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>


namespace blank {

Camera::Camera()
: fov(45.0f)
, aspect(1.0f)
, near_clip(0.1f)
, far_clip(256.0f)
, projection(glm::perspective(fov, aspect, near_clip, far_clip)) {

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


void Camera::UpdateProjection() {
	projection = glm::perspective(fov, aspect, near_clip, far_clip);
}

}
