#ifndef BLANK_CAMERA_HPP_
#define BLANK_CAMERA_HPP_

#include <glm/glm.hpp>


namespace blank {

class Camera {

public:
	Camera();

	void Viewport(int width, int height);
	void Viewport(int x, int y, int width, int height);

	void FOV(float f);
	void Aspect(float r);
	void Aspect(float w, float h);
	void Clip(float near, float far);

	const glm::mat4 &Projection() { return projection; }

private:
	void UpdateProjection();

private:
	float fov;
	float aspect;
	float near_clip;
	float far_clip;

	glm::mat4 projection;

};

}

#endif
