#ifndef BLANK_GRAPHICS_CAMERA_HPP_
#define BLANK_GRAPHICS_CAMERA_HPP_

#include "glm.hpp"


namespace blank {

class Camera {

public:
	Camera() noexcept;

	/// FOV in radians
	void FOV(float f) noexcept;
	void Aspect(float r) noexcept;
	void Aspect(float w, float h) noexcept;
	void Clip(float near, float far) noexcept;

	const glm::mat4 &Projection() const noexcept { return projection; }
	const glm::mat4 &View() const noexcept { return view; }
	void View(const glm::mat4 &v) noexcept;

private:
	void UpdateProjection() noexcept;

private:
	float fov;
	float aspect;
	float near;
	float far;

	glm::mat4 projection;
	glm::mat4 view;

};

}

#endif
