#ifndef BLANK_CAMERA_HPP_
#define BLANK_CAMERA_HPP_

#include <glm/glm.hpp>


namespace blank {

class Camera {

public:
	Camera() noexcept;

	void Viewport(int width, int height) noexcept;
	void Viewport(int x, int y, int width, int height) noexcept;

	/// FOV in radians
	void FOV(float f) noexcept;
	void Aspect(float r) noexcept;
	void Aspect(float w, float h) noexcept;
	void Clip(float near, float far) noexcept;

	const glm::mat4 &Projection() noexcept { return projection; }

private:
	void UpdateProjection() noexcept;

private:
	float fov;
	float aspect;
	float near_clip;
	float far_clip;

	glm::mat4 projection;

};

}

#endif
