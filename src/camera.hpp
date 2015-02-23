#ifndef BLANK_CAMERA_HPP_
#define BLANK_CAMERA_HPP_

#include <glm/glm.hpp>

#include "controller.hpp"
#include "geometry.hpp"


namespace blank {

class Camera
: public FPSController {

public:
	Camera();

	Camera(const Camera &) = delete;
	Camera &operator =(const Camera &) = delete;

	glm::mat4 MakeMVP(const glm::mat4 &m) const { return vp * m; }

	void Viewport(int width, int height);
	void Viewport(int x, int y, int width, int height);

	void FOV(float f);
	void Aspect(float r);
	void Aspect(float w, float h);
	void Clip(float near, float far);

	Ray Aim() const;

	const glm::mat4 &Projection() { return projection; }
	const glm::mat4 &View() { return view; }

	void Update(int dt);

private:
	void UpdateProjection();

private:
	float fov;
	float aspect;
	float near_clip;
	float far_clip;

	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 vp;

};

}

#endif
